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
    case UVCHAN_ERR_SELECT_FULL:
      return "maximum number of tags reached listening on select";
    case UVCHAN_ERR_SELECT_DUPLICATE_TAG:
      return "tag already exists on select structure";
    default:
      return "unknown";
  }
}
