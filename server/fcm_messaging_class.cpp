
// https://developers.google.com/identity/protocols/OAuth2ServiceAccount
// https://firebase.google.com/docs/cloud-messaging/send-message

#include "base64.h"
#include "fcm_messaging_class.hpp"
#include "file.h"
#include "log.h"
#include "sha256.h"

#include <fstream>
#include <cpp-json/json.h>
#include <curl/curl.h>
#include <openssl/pem.h>


// private methods
fcm_messaging_class::fcm_messaging_class (
        void)
{

}

int fcm_messaging_class::parse_service_account_json (
        void)
{
    int result = 0;
    m_email = "";
    m_pem_private_key = "";
    json::value obj;

    // open teh json file
    if (result == 0)
    {
        std::ifstream file ("service-account.json");
        if (file)
            obj = json::parse (file);
        else
        {
            DEBUG_LOG_ERROR ("failed to open service-account.json");
            result = -1;
        }
    }

    // extract the email address
    if (result == 0)
    {
        json::value email = obj ["client_email"];
        m_email = stringify (email, json::ESCAPE_UNICODE);
    }

    // extract the private key
    if (result == 0)
    {
        json::value key = obj ["private_key"];
        m_pem_private_key = stringify (key, json::ESCAPE_UNICODE);

        // get the first occurrence of endl
        size_t pos = m_pem_private_key.find ("\\n");
 
        // and remove them all
        while (pos != std::string::npos)
        {
            // Replace this occurrence of Sub String
            m_pem_private_key.replace (pos, 2, "\n");
            // Get the next occurrence from the current position
            pos = m_pem_private_key.find ("\\n", pos + 2);
        }

        // remove quotes
        m_pem_private_key = m_pem_private_key.substr (1, m_pem_private_key.size() - 2);
    }

    return result;
}

int fcm_messaging_class::prepare_jwt (
        void)
{
    int result = 0;
    m_jwt = "";
    unsigned int base64_length = 0;

    // get base64 url encoded header
    if (result == 0)
    {
        const char header_str [] =  "{\"alg\":\"RS256\",\"typ\":\"JWT\"}";
        char* header = base64url_encode ((unsigned char*)header_str, strlen (header_str), &base64_length);

        // append the header to the jws data
        m_jwt += header;
        m_jwt += ".";
        free (header);
    }

    // get the current and expiration (in one hour) time
    time_t curr_time = time (NULL);
    time_t exp_time = curr_time + 3600;

    // get base64 url encoded claim
    if (result == 0)
    {
        char message [1024];
        memset (message, 0, sizeof (message));
        snprintf (message,  sizeof (message) - 1,
                "{\"iss\":%s,\"scope\":\"https://www.googleapis.com/auth/firebase.messaging\",\
                \"aud\":\"https://www.googleapis.com/oauth2/v4/token\",\"exp\":%ld,\"iat\":%ld}",
                m_email.c_str () /*"firebase-adminsdk-9n8jv@shp-server.iam.gserviceaccount.com"*/, exp_time, curr_time);
        char* claim = base64url_encode ((unsigned char*)message, strlen (message), &base64_length);

        // append the claim to the jws data
        m_jwt += claim;
        free (claim);
    }

    // get a BIO
    BIO* bio = NULL;
    if (result == 0)
    {
        bio = BIO_new_mem_buf ((char*)m_pem_private_key.c_str (), -1);
        if (bio == NULL)
        {
            DEBUG_LOG_ERROR ("BIO_new_mem_buf failed\n");
            result = -1;
        }
    }

    // load the certificate
    RSA* rsakey = NULL;
    if (result == 0)
    {
        rsakey = PEM_read_bio_RSAPrivateKey (bio, NULL, 0, NULL);
        if (!RSA_check_key (rsakey))
        {
            DEBUG_LOG_ERROR ("failed to validate RSA key");
            result = -1;
        }
    }

    // RSASSA-PKCS1-V1_5-SIGN with SHA256 digest
    if (result == 0)
    {
        // get hash of the "header.claim" message
        unsigned char digest [SHA256_RESULT_LEN];
        sha256 ((unsigned char*)m_jwt.c_str (), m_jwt.size (), digest);

        unsigned int sign_len = 0;
        unsigned char* sign_buffer = (unsigned char*) malloc (RSA_size (rsakey));
        if (RSA_sign (NID_sha256, digest, SHA256_RESULT_LEN, sign_buffer, &sign_len, rsakey) == 1)
        {
            // get base64 url encoded signature
            char* signature = base64url_encode (sign_buffer, sign_len, &base64_length);

            // update jws data with signature
            m_jwt += ".";
            m_jwt += signature;
            free (signature);
        }
        else
        {
            DEBUG_LOG_ERROR ("failed to sigh jwt data");
            result = -1;
        }

        free (sign_buffer);
    }

    // add a grand type prefix to the curent jwt data
    const char grand_type_str [] = "grant_type=urn%3Aietf%3Aparams%3Aoauth%3Agrant-type%3Ajwt-bearer&assertion=";
    m_jwt = grand_type_str + m_jwt;

    // clean up
    if (rsakey)
        RSA_free (rsakey);

    if (bio)
        BIO_free(bio);

    return result;
}

int fcm_messaging_class::request_new_access_token (
        void)
{
    int result = 0;
    CURL* curl = nullptr;
    struct curl_slist* chunk = NULL;

    curl_global_init (CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init ();
    if (curl == NULL)
    {
        DEBUG_LOG_ERROR ("curl_easy_init failed");
        result = -1;
    }

    // set URL
    if (result == 0)
    {
        char google_url [] = "https://www.googleapis.com/oauth2/v4/token";
        CURLcode res = curl_easy_setopt (curl, CURLOPT_URL, google_url);
        if (res != CURLE_OK)
        {
            DEBUG_LOG_ERROR ("failed to set URL: %s", curl_easy_strerror (res));
            result = -1;
        }
    }

    // prepare header
    if (result == 0)
    {
        chunk = curl_slist_append (chunk, "Host: www.googleapis.com");
        chunk = curl_slist_append (chunk, "Content-Type: application/x-www-form-urlencoded");
        CURLcode res = curl_easy_setopt (curl, CURLOPT_HTTPHEADER, chunk);
        if (res != CURLE_OK)
        {
            DEBUG_LOG_ERROR ("failed to set HEADER: %s", curl_easy_strerror (res));
            result = -1;
        }
    }

    // prepare the jwt full string
    if (result == 0)
    {
        if (parse_service_account_json () != 0)
        {
            DEBUG_LOG_ERROR ("parse_service_account_json failed");
            result = -1;
        }
    }

    // prepare the jwt full string
    if (result == 0)
    {
        if (prepare_jwt () != 0)
        {
            DEBUG_LOG_ERROR ("prepare_jwt failed");
            result = -1;
        }
    }

    // prepare body
    if (result == 0)
    {
        CURLcode res = curl_easy_setopt (curl, CURLOPT_POSTFIELDS, m_jwt.c_str ());
        if (res != CURLE_OK)
        {
            DEBUG_LOG_ERROR ("failed to set BODY: %s", curl_easy_strerror (res));
            result = -1;
        }
    }

    // set write callback and callback data
    if (result == 0)
    {
        m_token_reply = "";

        curl_easy_setopt (curl, CURLOPT_WRITEDATA, this);
        CURLcode res = curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, request_new_access_token_callback);
        if (res != CURLE_OK)
        {
            DEBUG_LOG_ERROR ("failed to set callback: %s", curl_easy_strerror (res));
            result = -1;
        }
    }

    // send the POST request
    if (result == 0)
    {
        CURLcode res = curl_easy_perform (curl);
        if (res != CURLE_OK)
        {
            DEBUG_LOG_ERROR ("curl_easy_perform() failed: %s", curl_easy_strerror (res));
            result = -1;
        }
    }

    curl_slist_free_all (chunk);
    curl_easy_cleanup (curl);
    curl_global_cleanup ();

    return result;
}

unsigned int fcm_messaging_class::request_new_access_token_callback (
        const char* ptr,
        const unsigned int size,
        const unsigned int nmemb,
        fcm_messaging_class* p_base)
{
    p_base->update_new_access_token (ptr, size * nmemb);
    return size * nmemb;    
}

void fcm_messaging_class::update_new_access_token (
        const char* reply,
        const unsigned int length)
{
    //DEBUG_LOG_INFO ("%s", reply);

    m_token_reply += std::string (reply, length);
    std::size_t pos = m_token_reply.find ("\"expires_in\"");
    if (pos != std::string::npos)
    {
        DEBUG_LOG_INFO ("request send was successful");

        std::string str ("\"access_token\": \"");
        pos = m_token_reply.find (str);
        m_token_reply.erase (0, pos + str.size ());
        pos = m_token_reply.find ("\"", pos);
        m_token_reply.erase (pos, m_token_reply.size () - pos);

        //DEBUG_LOG_INFO ("%s\n", m_token_reply.c_str ());
        drop_file ("access_token");

        save_to_file ("access_token", m_token_reply.c_str (), m_token_reply.size ());
        return;
    }

    pos = m_token_reply.find ("\"error\"");
    if (pos != std::string::npos)
    {
        DEBUG_LOG_ERROR ("request wasn't successful");
        return;
    }
}

int fcm_messaging_class::get_access_token_from_cache (
        void)
{
    int result = 0;

    drop_old_file ("access_token", 3550);

    // do we have access_token file?
    if (!does_file_exist ("access_token"))
    {
        result = -1;
    }

    if (result == 0)
    {
        char* buffer = NULL;
        unsigned int size = 0;
        read_from_file ("access_token", (void**)&buffer, &size);
        m_access_token = std::string (buffer, size);
        free (buffer);

        if (m_access_token == "")
        {
            DEBUG_LOG_ERROR ("read access_token file failed %s", strerror (errno));
            result = -1;
        }
        else
        {
            DEBUG_LOG_INFO ("file access_token is valid");
        }
    }

    return result;
}

int fcm_messaging_class::send_message (
        void)
{
    int result = 0;
    CURL* curl = NULL;
    struct curl_slist* chunk = NULL;

    curl_global_init (CURL_GLOBAL_DEFAULT);

    if (result == 0)
    {
        curl = curl_easy_init ();
        if (curl == NULL)
        {
            DEBUG_LOG_ERROR ("curl_easy_init failed");
            result = -1;
        }
    }

    //curl_easy_setopt (curl, CURLOPT_STDERR, filep);
    //curl_easy_setopt (curl, CURLOPT_VERBOSE, 1L);

    // set URL
    if (result == 0)
    {
        char fcm_url [] = "https://fcm.googleapis.com/v1/projects/shp-server/messages:send";
        CURLcode res = curl_easy_setopt (curl, CURLOPT_URL, fcm_url);
        if (res != CURLE_OK)
        {
            DEBUG_LOG_ERROR ("failed to set URL: %s", curl_easy_strerror (res));
            result = -1;
        }
    }

    // prepare header
    if (result == 0)
    {
        std::string auth = "Authorization: Bearer " + m_access_token;
        chunk = curl_slist_append (chunk, auth.c_str ());
        chunk = curl_slist_append (chunk, "Content-Type: application/json; UTF-8");
        CURLcode res = curl_easy_setopt (curl, CURLOPT_HTTPHEADER, chunk);
        if (res != CURLE_OK)
        {
            DEBUG_LOG_ERROR ("failed to set HEADER: %s", curl_easy_strerror (res));
            result = -1;
        }
    }

    // prepare body
    if (result == 0)
    {
        std::string message;
        message =  "{";
        message +=      "\"message\":{";
        message +=      "\"topic\":\"global\",";
        message +=      "\"notification\":{";
        message +=          "\"body\":\"" + m_body + "\",";
        message +=          "\"title\":\"" + m_title + "\",";
        message +=      "}";
        message +=  "}";
        message += "}";
        CURLcode res = curl_easy_setopt (curl, CURLOPT_COPYPOSTFIELDS, message.c_str ());
        if (res != CURLE_OK)
        {
            DEBUG_LOG_ERROR ("failed to set BODY: %s\n", curl_easy_strerror (res));
            result = -1;
        }
    }

    // set write callback and callback data
    if (result == 0)
    {
        m_fcm_reply = "";

        curl_easy_setopt (curl, CURLOPT_WRITEDATA, this);
        CURLcode res = curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, send_message_callback);
        if (res != CURLE_OK)
        {
            DEBUG_LOG_ERROR ("failed to set callback: %s", curl_easy_strerror (res));
            result = -1;
        }
    }

    // send the POST request
    if (result == 0)
    {
        CURLcode res = curl_easy_perform (curl);
        if (res != CURLE_OK)
        {
            DEBUG_LOG_ERROR ("curl_easy_perform failed: %s", curl_easy_strerror (res));
            result = -1;
        }
    }

    curl_slist_free_all (chunk);
    curl_easy_cleanup (curl);
    curl_global_cleanup ();

    return result;
}

unsigned int fcm_messaging_class::send_message_callback (
        const char* ptr,
        const unsigned int size,
        const unsigned int nmemb,
        fcm_messaging_class* p_base)
{
    p_base->update_send_status (ptr, size * nmemb);
    return size * nmemb;
}

void fcm_messaging_class::update_send_status (
        const char* reply,
        const unsigned int length)
{
    //DEBUG_LOG_INFO ("%s\n", reply);

    m_fcm_reply += std::string (reply, length);
    std::size_t pos = m_fcm_reply.find ("\"name\"");
    if (pos != std::string::npos)
    {
        DEBUG_LOG_INFO ("send was successful");
        return;
    }

    pos = m_fcm_reply.find ("\"error\"");
    if (pos != std::string::npos)
    {
        DEBUG_LOG_ERROR ("send wasn't successful");
        return;
    }
}


// public methods
void fcm_messaging_class::register_message (
    std::string title,
    std::string body)
{
    m_title = title;
    m_body = body;

    m_access_token = "";

    int result = get_access_token_from_cache ();

    if (result != 0)
        result = request_new_access_token ();

    if (result == 0 && m_access_token == "")
        result = get_access_token_from_cache ();

    if (result == 0)
        send_message ();
}
