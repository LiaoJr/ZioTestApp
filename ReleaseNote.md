V2.0.3.240626
Release date: 2024.06.26
Release log: 
- 修复文件召回时分片上传文件示例中文件名字不一致的问题

V2.0.3.240418
Release date: 2024.04.18
Release log: 
- 修复帮助文档中图片未显示的问题
- 示例中多个client_req_servfunc只保留一个，其他注释掉
- 修改STR_TOPIC_UNBIND_SUBDEV的bug

V2.0.3.240327
Release date: 2024.03.27
Release log: 
- 安装包下载兼容新旧版本ZWS
- 安装包下载优化HTTP分片相关的代码

V2.0.3.240321
Release date: 2024.03.21
Release log: 
- package使用的zlab.zlgcloud.com修正为zws.zlgcloud.com
- 公钥证书更新为./bin/cert.cer，获得更长的过期时间

V2.0.3.240311
Release date: 2024.03.11
Release log: 
- 新增qt工程示例，并在Windows和Linux平台测试通过

V2.0.3.240223
Release date: 2024.02.23
Release log: 
- 添加设备加密通信的demo和文档
- 文件格式从UTF-8修改为UTF-8 BOM，兼容Windows平台的编译

V2.0.2.240125
Release date: 2024.01.25
Release log: 
- 优化固件下载的注释说明

V2.0.2.231220
Release date: 2023.12.20
Release log: 
- 优化重连时get_mqtt_info_impl中内存未释放的问题
- 优化程序结束时未取消订阅造成的内存未释放的问题
- enableAutoReconnect默认值由false修改为true，默认开启重连
- 优化while循环退出的逻辑

V2.0.1.231117
Release date: 2023.11.17
Release log: 
- 添加Industry_ES和Home_ES示例

V2.0.1.230913
Release date: 2023.09.13
Release log: 
- 修复编译时的告警

V2.0.1.230911
Release date: 2023.09.11
Release log: 
- 内置对em-500、em-1000等的支持

V2.0.1.230627
Release date: 2023.06.27
Release log: 
- 连接mqtt加密端口测试通过

V2.0.1.230607
Release date: 2023.06.07
Release log: 
- devide_expl完善命令mydev_req_report_exec

V2.0.1.230605
Release date: 2023.06.05
Release log: 
- 文档中自定义命令完善mtype的说明；
- 透传命令章节细化为pass_through和raw透传两种情况

V2.0.1.230602
Release date: 2023.06.02
Release log: 设置默认连接新版线上服务

V2.0.1.230527
Release date: 2023.05.27
Release log: 完善支持ZWS_R2文档

V1.1.3.210923
Release date: 2021.09.23
Release log: 修复params_append_string键值对占用大小判断错误

V1.1.3.210914
Release date: 2021.09.14
Release log: 解析http header前把相关参数置为0

V1.1.3.210721
Release date: 2021.07.21
Release log: 新增设备影子的Function

V1.1.2.210623
Release date: 2021.06.23
Release log: 修正http分块传输协议引起的http响应数据出现多余字符的问题

V1.1.1.210513
Release date: 2021.05.13
Release log: 新增http文件上传功能

V1.1.0.200818
Release date: 2020.08.18
Release log: 添加获取短地址映射的示例

V1.1.0.200721
Release date: 2020.07.21
Release log: http兼容阿里云部署和私有化部署

V1.0.9.200424
Release date: 2020.04.24
Release log: 支持白名单相关命令

V1.0.9.200218
Release date: 2020.02.18
Release log: 新增安装包下载

V1.0.8.200416
Release date: 2020.04.16
Release log: 新增set_multicast组播命令

V1.0.7.191207
Release date: 2019.12.07
Release log: 支持网关获取子设备信息

V1.0.6.191008
Release date: 2019.10.08
Release log:支持设备上线时获取固件信息

V1.0.5.190805
Release date: 2019.08.05
Release log:修复错误释放内存的问题
