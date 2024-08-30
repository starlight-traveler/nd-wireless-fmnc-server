#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

void initialize_ssl()
{
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

SSL_CTX *create_context()
{
    const SSL_METHOD *method = TLS_server_method();
    SSL_CTX *ctx = SSL_CTX_new(method);
    if (!ctx)
    {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    return ctx;
}

void cleanup_openssl()
{
    EVP_cleanup();
    ERR_free_strings();
}

void configure_context(SSL_CTX *ctx)
{
    SSL_CTX_set_ecdh_auto(ctx, 1);
    // Set the key and cert
    if (SSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

int main()
{
    initialize_ssl();
    SSL_CTX *ctx = create_context();
    configure_context(ctx);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    listen(sockfd, 1);

    while (1)
    {
        struct sockaddr_in addr;
        uint len = sizeof(addr);
        int client = accept(sockfd, (struct sockaddr *)&addr, &len);
        SSL *ssl = SSL_new(ctx);
        SSL_set_fd(ssl, client);

        if (SSL_accept(ssl) <= 0)
        {
            ERR_print_errors_fp(stderr);
        }
        else
        {
            char buffer[1024] = {0};
            SSL_read(ssl, buffer, sizeof(buffer));
            std::cout << "Client says: " << buffer << std::endl;
            SSL_write(ssl, buffer, strlen(buffer));
        }

        SSL_free(ssl);
        close(client);
    }

    close(sockfd);
    SSL_CTX_free(ctx);
    cleanup_openssl();
    return 0;
}