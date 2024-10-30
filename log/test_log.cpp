#include <iostream>
#include "log.h"

int main() {
    // 初始化日志系统
    if (!Log::get_instance()->init("test_log.txt", 0, 8192, 5000000, 1000)) {
        std::cerr << "Failed to initialize log system." << std::endl;
        return 1;
    }

    // 测试写入不同级别的日志
    Log::get_instance()->write_log(DEBUG, "This is a debug message.");
    Log::get_instance()->write_log(INFO, "This is an info message.");
    Log::get_instance()->write_log(WARN, "This is a warning message.");
    Log::get_instance()->write_log(ERROR, "This is an error message.");

    // 模拟接收原始日志
    Log::get_instance()->receiveLog("[info]: Device123 Operation successful.");

    // 刷新日志
    Log::get_instance()->flush();

    std::cout << "Logs written successfully." << std::endl;
    return 0;
}
