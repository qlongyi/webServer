演示
----------

<div align=center><img src="https://github.com/qinguoyi/TinyWebServer/blob/master/root/login.gif" height="429"/> </div>



快速运行
------------

* 测试前确认已安装MySQL数据库

    ```C++
    // 建立yourdb库
    create database yourdb;

    // 创建user表
    USE yourdb;
    CREATE TABLE user(
        name char(50) NULL,
        password char(50) NULL
    )ENGINE=InnoDB;

    // 添加数据
    INSERT INTO user(name, password) VALUES('name', 'password');
    ```

* -p，自定义端口号
	
	* 默认9527
* -l，选择日志写入方式，默认同步写入
	* 0，同步写入
	* 1，异步写入
* -m，listenfd和connfd的模式组合，默认使用LT + LT
	* 0，表示使用LT + LT
	* 1，表示使用LT + ET
    * 2，表示使用ET + LT
    * 3，表示使用ET + ET
* -o，优雅关闭连接，默认不使用
	* 0，不使用
	* 1，使用
* -s，数据库连接数量
	
	* 默认为20
* -t，线程数量
	
	* 默认为20
* -c，关闭日志，默认打开
	* 0，打开日志
	* 1，关闭日志
* -a，选择反应堆模型，默认Proactor
	* 0，Proactor模型
	* 1，Reactor模型
