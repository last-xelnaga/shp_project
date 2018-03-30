
// https://developers.google.com/identity/protocols/OAuth2ServiceAccount
// https://firebase.google.com/docs/cloud-messaging/send-message

#include "fcm_messaging_class.hpp"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <curl/curl.h>
#include <openssl/pem.h>

#include "base64.h"
#include "sha256.h"


char header_str [] =  "{\"alg\":\"RS256\",\"typ\":\"JWT\"}";
char grand_type_str [] = "grant_type=urn%3Aietf%3Aparams%3Aoauth%3Agrant-type%3Ajwt-bearer&assertion=";

static char grant_type [2048];

static char* get_jwt (
        void)
{
    memset (grant_type, 0, sizeof (grant_type));

    strcpy (grant_type, grand_type_str);
    unsigned int start = strlen (grant_type);
    unsigned int curr = start;
    
    unsigned int base64_length = 0;
    char* header = base64_encode ((unsigned char*)header_str, strlen (header_str), &base64_length);
    convert_to_base64url (header, &base64_length);

    strcpy (&grant_type [curr], header);
    curr += base64_length;
    free (header);
    
    strcpy (&grant_type [curr], ".");
    curr ++;


    char message [1024];
    memset (message, 0, sizeof (message));

    time_t now = time (NULL);
    snprintf (message,  sizeof (message) - 1,
    "{\
    \"iss\":\"%s\",\
    \"scope\":\"https://www.googleapis.com/auth/firebase.messaging\",\
    \"aud\":\"https://www.googleapis.com/oauth2/v4/token\",\
    \"exp\":%ld,\
    \"iat\":%ld\
    }",
    "firebase-adminsdk-9n8jv@shp-server.iam.gserviceaccount.com", now + 3600, now);

    char* claim = base64_encode ((unsigned char*)message, strlen (message), &base64_length);
    convert_to_base64url (claim, &base64_length);

    strcpy (&grant_type [curr], claim);
    curr += base64_length;
    free (claim);

    // RSASSA-PKCS1-V1_5-SIGN with SHA256
    unsigned char digest [SHA256_RESULT_LEN];
    sha256 ((unsigned char*)&grant_type [start], curr - start, digest);

    OpenSSL_add_all_algorithms();

    EVP_PKEY* privkey = EVP_PKEY_new();
    FILE* f = fopen ("priv_key.pem", "rb");
    if (!PEM_read_PrivateKey (f, &privkey, NULL, NULL))
    {
        printf ("Error loading Private Key File.\n");
    }
    fclose (f);

    RSA* rsakey = EVP_PKEY_get1_RSA(privkey);
    if (RSA_check_key (rsakey))
    {
        printf ("RSA key is valid.\n");
    }
    else
    {
        printf ("Error validating RSA key.\n");
    }


    unsigned char* sign_buffer = (unsigned char*) malloc (RSA_size (rsakey));
    unsigned int sign_len = 0;

    if (RSA_sign (NID_sha256, digest, SHA256_RESULT_LEN, sign_buffer, &sign_len, rsakey) != 1)
    {
        printf ("RSA_sign failed.\n");
    }

EVP_PKEY_free(privkey);

    char* signature = base64_encode (sign_buffer, sign_len, &base64_length);
    convert_to_base64url (signature, &base64_length);

    strcpy (&grant_type [curr], ".");
    curr ++;

    strcpy (&grant_type [curr], signature);
    curr += base64_length;
    free (signature);

    base64_cleanup ();

    return grant_type;
}

// private methods
fcm_messaging_class::fcm_messaging_class (
        void)
{
    
}

int fcm_messaging_class::request_new_access_token (
        void)
{
    int result = 0;
    CURL* curl = nullptr;
    struct curl_slist* chunk = nullptr;

    curl_global_init (CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init ();
    if (curl == NULL)
    {
        printf ("curl_easy_init failed\n");
        result = -1;
    }

    // set URL
    if (result == 0)
    {
        char google_url [] = "https://www.googleapis.com/oauth2/v4/token";
        CURLcode res = curl_easy_setopt (curl, CURLOPT_URL, google_url);
        if (res != CURLE_OK)
        {
            printf ("failed to set URL: %s\n", curl_easy_strerror (res));
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
            printf ("failed to set HEADER: %s\n", curl_easy_strerror (res));
            result = -1;
        }
    }

    // prepare body
    if (result == 0)
    {
        CURLcode res = curl_easy_setopt (curl, CURLOPT_POSTFIELDS, get_jwt ());
        if (res != CURLE_OK)
        {
            printf ("failed to set BODY: %s\n", curl_easy_strerror (res));
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
            printf ("failed to set callback: %s\n", curl_easy_strerror (res));
            result = -1;
        }
    }

    // send the POST request
    if (result == 0)
    {
        CURLcode res = curl_easy_perform (curl);
        if (res != CURLE_OK)
        {
            printf ("curl_easy_perform() failed: %s\n", curl_easy_strerror (res));
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
    printf ("%s\n", reply);

    m_token_reply += std::string (reply, length);
    std::size_t pos = m_token_reply.find ("\"expires_in\"");
    if (pos != std::string::npos)
    {
        printf ("request send was successful\n");

        std::string str ("\"access_token\": \"");
        pos = m_token_reply.find (str);
        m_token_reply.erase (0, pos + str.size ());
        pos = m_token_reply.find ("\"", pos);
        m_token_reply.erase (pos, m_token_reply.size () - pos);

        printf ("%s\n", m_token_reply.c_str ());
        unlink ("access_token");

        FILE* f = fopen ("access_token", "wb");
        if (f != nullptr)
        {
            fwrite (m_token_reply.c_str (), m_token_reply.size (), 1, f);
            fclose (f);
        }
        return;
    }

    pos = m_token_reply.find ("\"error\"");
    if (pos != std::string::npos)
    {
        printf ("request wasn't successful\n");
        return;
    }
}

int fcm_messaging_class::get_access_token_from_cache (
        void)
{
    int result = 0;

    // do we have access_token file?
    if (result == 0)
    {
        struct stat buffer;   
        if (stat ("access_token", &buffer) != 0)
        {
            printf ("file access_token does not exist\n");
            result = -1;
        }
        else
        {
            time_t now = time (NULL);
            printf ("now %ld, file %ld, diff %ld\n", now, buffer.st_ctime, now - buffer.st_ctime);
            if (now > buffer.st_ctime + 3550)
            {
                printf ("file access_token is old\n");
                unlink ("access_token");
                result = -1;
            }
        }
    }

    if (result == 0)
    {
        FILE* f = fopen ("access_token", "rb");
        if (f != nullptr)
        {
            fseek (f, 0, SEEK_END);
            long fsize = ftell (f);
            fseek (f, 0, SEEK_SET);

            char* buffer = new char [fsize];
            fread (buffer, fsize, 1, f);
            fclose (f);

            m_access_token = std::string (buffer, fsize);
            delete buffer;
        }
        
        if (m_access_token == "")
        {
            printf ("read access_token file failed %s\n", strerror (errno));
            result = -1;
        }
        else
        {
            printf ("file access_token is valid\n");
        }
    }

    return result;
}

int fcm_messaging_class::send_message (
        void)
{
    int result = 0;
    CURL* curl = nullptr;
    struct curl_slist* chunk = nullptr;

    curl_global_init (CURL_GLOBAL_DEFAULT);

    if (result == 0)
    {
        curl = curl_easy_init ();
        if (curl == nullptr)
        {
            printf ("curl_easy_init failed\n");
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
            printf ("failed to set URL: %s\n", curl_easy_strerror (res));
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
            printf ("failed to set HEADER: %s\n", curl_easy_strerror (res));
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
            printf ("failed to set BODY: %s\n", curl_easy_strerror (res));
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
            printf ("failed to set callback: %s\n", curl_easy_strerror (res));
            result = -1;
        }
    }

    // send the POST request
    if (result == 0)
    {
        CURLcode res = curl_easy_perform (curl);
        if (res != CURLE_OK)
        {
            printf ("curl_easy_perform failed: %s\n", curl_easy_strerror (res));
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
    printf ("%s\n", reply);

    m_fcm_reply += std::string (reply, length);
    std::size_t pos = m_fcm_reply.find ("\"name\"");
    if (pos != std::string::npos)
    {
        printf ("send was successful\n");
        return;
    }

    pos = m_fcm_reply.find ("\"error\"");
    if (pos != std::string::npos)
    {
        printf ("send wasn't successful\n");
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
