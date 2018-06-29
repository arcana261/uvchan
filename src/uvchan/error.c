#include <uvchan/error.h>

const char* uvchan_strerr(int code) {
  switch (code) {
    case UVCHAN_ERR_SUCCESS:
      return "success";
    case UVCHAN_ERR_QUEUE_EMPTY:
      return "queue is empty";
    case UVCHAN_ERR_QUEUE_FULL:
      return "queue is full";
      case UVCHAN_ERR_CHANNEL_CLOSED:
      return "channel is closed";
    default:
      return "unknown";
  }
}
