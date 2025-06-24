#### APP登录

APP(如CanMaster)也可以直接与MQTT服务器通讯，登录流程如下：
1. 到API网关注册用户并绑定相应的设备。
2. 从API网关获取所有设备接口，获取所需设备列表，后续登录MQTT服务器时会用到。
3. 使用用户和密码登录认证服务器[https://auth.zlgcloud.com:8143/userLogin](https://auth.zlgcloud.com:8143/userLogin)，获取MQTT的相关信息。
4. 后续流程同设备登录流程一致，只需在登录MQTT服务器时，把**ClientId**设置为格式化字符串"appuser:${username}"的值即可。

**当前最大可操作设备数（即MQTT登录时的设备列表长度）最大为20。**

> 参考API文档：https://auth.zlgcloud.com:8143/api-explorer