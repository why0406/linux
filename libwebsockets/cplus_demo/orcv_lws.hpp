#ifndef _ORCVLWS_
#define _ORCVLWS_
#include <libwebsockets.h>

int
callback_http(struct lws *wsi, enum lws_callback_reasons reason,
                  void *user, void *in, size_t len);

int
callback_demo(struct lws *wsi, enum lws_callback_reasons reason,
                  void *user, void *in, size_t len);

static struct lws_protocols protocols_demo[] = {
    /* first protocol must always be HTTP handler */
    {
        .name = "http-only",
        .callback = callback_http,
        .per_session_data_size = 0
    },
    {
        .name = "demo-protocol",        // protocol name - client and server should assign same protocol
        .callback = callback_demo,      // callback function name
        .per_session_data_size = 0,     // ?
        .rx_buffer_size = (1024*512)    // ?
    },
    {
        NULL, NULL, 0                   // End of list, must have add this struct array element
    }
};

class Orcvlws {
    public:

    private:
    public:
        //============================
        // local websocket parameter
        //============================
        int    port;                                   // 通信端口
        struct lws_context *context;
        struct lws_context_creation_info context_info; // 用于填充创建vhost或者context时所需参数
        //===========================================================
        // if local websocket is client, need set service parameter
        //===========================================================
        const char* addr;                              // 客户端要连接的服务端地址
        struct lws *conn_wsi;                          // 客户端与服务端建立的连接
        struct lws_client_connect_info conn_info;      // 客户端要连接的服务端信息结构体


    public:
        Orcvlws() = delete;
        Orcvlws(uint32_t port, struct lws_protocols protocols[]);                        // as service
        Orcvlws(uint32_t port, const char *addr, struct lws_protocols protocols[]);      // as client
        ~Orcvlws();

        int  CreateLocalContext();
        int  ConnService();
        void WaitClient();
        void WaitService();
        void SetSignal();                              // 用于临时退出

    private:
};

#endif
