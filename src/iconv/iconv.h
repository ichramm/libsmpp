#ifndef OPENSMPP_ICONV_ICONV_H__
#define OPENSMPP_ICONV_ICONV_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

typedef void* iconv_t;
iconv_t iconv_open(const char *tocode, const char *fromcode);
int iconv_close(iconv_t cd);
unsigned iconv(iconv_t cd, const char **inbuf, unsigned *inbytesleft, char **outbuf, unsigned *outbytesleft);

#ifdef __cplusplus
}
#endif

#endif // OPENSMPP_ICONV_ICONV_H__
