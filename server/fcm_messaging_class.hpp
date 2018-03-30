
#include <string>

//https://stackoverflow.com/questions/1008019/c-singleton-design-pattern

class fcm_messaging_class
{
private:
    std::string m_access_token;
    std::string m_title;
    std::string m_body;
    std::string m_token_reply;
    std::string m_fcm_reply;

public:
    static fcm_messaging_class& get_instance (
            void)
    {
        static fcm_messaging_class instance;
        return instance;
    }
private:
    fcm_messaging_class (
            void);

    int request_new_access_token (
            void);

    static unsigned int request_new_access_token_callback (
            const char* ptr,
            const unsigned int size,
            const unsigned int nmemb,
            fcm_messaging_class* p_base);

    void update_new_access_token (
            const char* reply,
            const unsigned int length);

    int get_access_token_from_cache (
            void);


    // send message routines
    int send_message (
            void);

    static unsigned int send_message_callback (
            const char* ptr,
            const unsigned int size,
            const unsigned int nmemb,
            fcm_messaging_class* p_base);

    void update_send_status (
            const char* reply,
            const unsigned int length);

public:
    fcm_messaging_class (fcm_messaging_class const&) = delete;
    void operator=(fcm_messaging_class const&) = delete;

    void register_message (
        std::string title,
        std::string body);

};
