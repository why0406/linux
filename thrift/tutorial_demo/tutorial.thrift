/** 
 * Thrift引用其他thrift文件, 这些文件可以从当前目录中找到, 或者使用-I的编译器参数指示.
 * 引入的thrift文件中的对象, 使用被引入thrift文件的名字作为前缀, 例如shared.SharedStruct.
 */
include "shared.thrift"

namespace cpp tutorial

// 定义别名
typedef i32 MyInteger

/**
 * 定义常量. 复杂的类型和结构体使用JSON表示法. 
 */
const i32 INT32CONSTANT = 9853
const map<string,string> MAPCONSTANT = {'hello':'world', 'goodnight':'moon'}

/**
 * 枚举是32位数字, 如果没有显式指定值,从1开始.
 */
enum Operation {
  ADD = 1,
  SUBTRACT = 2,
  MULTIPLY = 3,
  DIVIDE = 4
}

/**
 * 结构体由一组成员来组成, 一个成员包括数字标识符, 类型, 符号名, 和一个可选的默认值.
 * 成员可以加"optional"修饰符, 用来表明如果这个值没有被设置, 那么他们不会被串行化到
 * 结果中. 不过这个在有些语言中, 需要显式控制.
 */
struct Work {
  1: i32 num1 = 0,
  2: i32 num2,
  3: Operation op,
  4: optional string comment,
}

// 结构体也可以作为异常
exception InvalidOperation {
  1: i32 whatOp,
  2: string why
}

/**
 * 服务需要一个服务名, 加上一个可选的继承, 使用extends关键字 
 */
service Calculator extends shared.SharedService {

  /**
　 * 方法定义和C语言一样, 有返回值, 参数或者一些它可能抛出的异常, 参数列表和异常列表的
　 * 写法与结构体中的成员列表定义一致. 
　 */

   void ping(),

   i32 add(1:i32 num1, 2:i32 num2),

   i32 calculate(1:i32 logid, 2:Work w) throws (1:InvalidOperation ouch),

    /**
 　　* 这个方法有oneway修饰符, 表示客户段发送一个请求, 然后不会等待回应. Oneway方法
　　 * 的返回值必须是void
　　 */
   oneway void zip()

}
