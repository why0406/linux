/*
 * 这个Thrift文件包含一些共享定义 
 */

namespace cpp shared

struct SharedStruct {
  1: i32 key
  2: string value
}

service SharedService {
  SharedStruct getStruct(1: i32 key)
}
