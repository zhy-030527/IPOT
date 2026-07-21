# 智慧盆栽项目 / 智能浇水系统

## Description
本项目是一个基于 STM32 的智能浇水系统（智慧盆栽）。项目集成了多维度的环境监测（温湿度、光照、土壤湿度），结合 FreeRTOS 实时操作系统，并通过 ESP8266 模块（WIFI + MQTT 协议）实现了与微信小程序的云端物联网互联。内部包含 STM32 硬件控制代码、微信小程序端代码，后续还将开源配套的 SolidWorks 3D 外壳模型文件。

## Software Architecture
本项目软件架构主要分为以下四个核心部分：

1. **第一部分：基础硬件与驱动搭建**
   * **主控芯片**：STM32F103C8T6
   * **DHT11 温湿度传感器**：采用 I2C/单总线通讯，实时监测环境温湿度。
   * **BH1750 光照传感器**：采用 I2C 通讯，采集环境光照强度。
   * **电容式土壤湿度传感器**：采用 ADC 通讯，精准获取土壤水分。
   * **继电器与 LED 控制**：采用 GPIO 直接控制，用于驱动水泵浇灌与灯光。
   * **ESP8266 无线传输模块**：采用 USART 串口通讯，负责网络交互。

2. **第二部分：FreeRTOS 任务搭建**
   * **多任务并发**：创建多个独立的 Task 处理数据采集、网络通信与硬件控制。
   * **任务间通信**：使用**队列 (Queue)** 和 **信号量 (Semaphore)** 进行任务间的数据流转与同步。
   * **软件定时器**：系统预留了软件定时器架构（待后续具体场景接入）。

3. **第三部分：物联网连接**
   * **网络接入**：通过 ESP8266 实现 WiFi 连接。
   * **协议通信**：基于 MQTT 协议实现设备端与微信小程序端的双向通信。
   * **数据处理**：使用 cJSON 进行云端下发与设备上报数据的解析。

4. **第四部分：创新点与演进计划**
   * **增加摄像头**：实现对盆栽的实时视觉监控。
   * **电源管理**：优化休眠策略，解决系统的功耗问题。
   * **OTA 升级**：通过 MQTT 远程推送固件包，实现单片机空中升级。

## Installation
1. 硬件连线：参考项目中提供的原理图，将 STM32F103C8T6 与各传感器（DHT11、BH1750、土壤传感器）、水泵继电器及 ESP8266 进行连接。
2. 固件编译：使用 Keil MDK 或 STM32CubeIDE 打开工程，编译无误后烧录至 STM32 开发板。
3. 小程序部署：使用微信开发者工具打开小程序源码，在配置中替换为您自己的 MQTT 服务器地址及设备鉴权参数。
4. 模型打印（后续支持）：使用 SolidWorks 软件打开提供的三维模型文件，导出 STL 格式进行 3D 打印。

## Instructions
1. **设备上电**：系统通电后，ESP8266 将自动连接预设的 WIFI，并登录 MQTT 服务器。
2. **数据监控**：传感器任务会周期性采集当前环境的温湿度、光照及土壤水分，并通过队列发送给网络任务，最终上报至微信小程序。
3. **自动浇灌**：当电容式土壤湿度传感器检测到土壤过于干燥时，系统可自动触发继电器开启水泵浇水。
4. **远程控制**：用户可随时打开微信小程序查看实时环境数据，并下发 JSON 指令手动控制水泵或 LED 灯的开关。

## Contribution
1. Fork the repository
2. Create Feat_xxx branch
3. Commit your code
4. Create Pull Request

## Gitee Feature
* You can use Readme_XXX.md to support different languages, such as Readme_en.md, Readme_zh.md
* Gitee blog [blog.gitee.com](https://blog.gitee.com)
* Explore open source project [https://gitee.com/explore](https://gitee.com/explore)
* The most valuable open source project [GVP](https://gitee.com/gvp)
* The manual of Gitee [https://gitee.com/help](https://gitee.com/help)
* The most popular members  [https://gitee.com/gitee-stars/](https://gitee.com/gitee-stars/)
