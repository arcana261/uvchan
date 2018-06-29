#ifndef UVCHAN_ERROR_H__
#define UVCHAN_ERROR_H__

typedef enum _uvchan_error_t {
  UVCHAN_ERR_SUCCESS = 0,
  UVCHAN_ERR_QUEUE_FULL,
  UVCHAN_ERR_QUEUE_EMPTY,
  UVCHAN_ERR_CHANNEL_CLOSED,
  _UVCHAN_ERR_COUNT
} uvchan_error_t;

const char* uvchan_strerr(int code);

#endif  // UVCHAN_ERROR_H__
