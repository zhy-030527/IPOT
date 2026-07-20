<template>
	<view class="page-wrapper">
		<!-- 🌟 修改：加入 isBannerDismissed 判断，并增加关闭按钮 -->
		<view class="warning-banner" v-if="hasDisconnectedSensor && !isBannerDismissed">
			<view class="warning-content">
				<text class="warning-icon">⚠️</text>
				<text class="warning-text">系统警告：检测到部分传感器断开连接！</text>
			</view>
			<text class="close-icon" @click="dismissBanner">✕</text>
		</view>

		<view class="container">
			<view class="header">
				<text class="title">IPot</text>
				<text class="subtitle">实时监测与智能控制</text>
			</view>

			<view class="card sensor-card">
				<view class="card-header">
					<text class="card-title">实时环境数据</text>
				</view>
				<view class="data-grid">
					<view class="data-item">
						<text class="icon">🌡️</text>
						<!-- 🌟 针对温度的断链处理 -->
						<text v-if="temp == -1" class="disconnect-text">Disconnect</text>
						<text v-else class="value">{{temp}} <text class="unit">℃</text></text>
						<text class="label">温度</text>
					</view>
					<view class="data-item">
						<text class="icon">💧</text>
						<!-- 🌟 针对湿度的断链处理 -->
						<text v-if="humi == -1" class="disconnect-text">Disconnect</text>
						<text v-else class="value">{{humi}} <text class="unit">%</text></text>
						<text class="label">环境湿度</text>
					</view>
					<view class="data-item">
						<text class="icon">☀️</text>
						<!-- 🌟 针对光照的断链处理 -->
						<text v-if="light == -1" class="disconnect-text">Disconnect</text>
						<text v-else class="value">{{light}} <text class="unit">lux</text></text>
						<text class="label">光照强度</text>
					</view>
					<view class="data-item">
						<text class="icon">🌱</text>
						<!-- 🌟 针对土壤湿度的断链处理 -->
						<text v-if="mois == -1" class="disconnect-text">Disconnect</text>
						<text v-else class="value">{{mois}} <text class="unit">%</text></text>
						<text class="label">土壤湿度</text>
					</view>
				</view>
			</view>

			<view class="card status-card">
				<view class="card-header">
					<text class="card-title">⚙️ 设备运行状态</text>
				</view>
				<view class="status-container">
					<view class="status-item">
						<text class="status-label">照明补光灯</text>
						<view class="status-badge" :class="{ 'active': ledStatus == 1 }">
							<text class="badge-dot"></text>
							<text>{{ ledStatus == 1 ? '工作中' : '已关闭' }}</text>
						</view>
					</view>
					<view class="divider-v"></view> 
					<view class="status-item">
						<text class="status-label">灌溉水泵</text>
						<view class="status-badge" :class="{ 'active': pumpStatus == 1 }">
							<text class="badge-dot"></text>
							<text>{{ pumpStatus == 1 ? '喷淋中' : '已关闭' }}</text>
						</view>
					</view>
				</view>
			</view>

			<view class="card control-card">
				<view class="card-header">
					<text class="card-title">💧 灌溉系统</text>
				</view>
				<view class="control-list">
					<view class="control-item">
						<view class="item-info">
							<text class="item-name">自动灌溉模式</text>
							<text class="item-desc">低于设定阈值自动浇水</text>
						</view>
						<switch :checked="autoIrrigation" @change="switchAutoIrrigation" color="#007aff" />
					</view>
					<view class="divider"></view>
					<view class="control-item">
						<view class="item-info">
							<text class="item-name" :class="{ 'disabled-text': autoIrrigation }">手动水泵开关</text>
						</view>
						<switch :checked="manualIrrigation" :disabled="autoIrrigation" @change="switchManualIrrigation" color="#34c759" />
					</view>
				</view>
			</view>

			<view class="card control-card">
				<view class="card-header">
					<text class="card-title">☀️ 照明系统</text>
				</view>
				<view class="control-list">
					<view class="control-item">
						<view class="item-info">
							<text class="item-name">自动补光模式</text>
							<text class="item-desc">光线暗时自动打开植物灯</text>
						</view>
						<switch :checked="autoLighting" @change="switchAutoLighting" color="#f0ad4e" />
					</view>
					<view class="divider"></view>
					<view class="control-item">
						<view class="item-info">
							<text class="item-name" :class="{ 'disabled-text': autoLighting }">手动补光开关</text>
						</view>
						<switch :checked="manualLighting" :disabled="autoLighting" @change="switchManualLighting" color="#ff3b30" />
					</view>
				</view>
			</view>

		</view>
	</view>
</template>

<script>
// 🌟 修复报错：将 ES6 的 import 替换为小程序兼容性更好的 require 语法
const { createCommonToken } = require('@/key.js');

export default {
	data() {
		return {
			// 传感器数据
			temp: '25',
			humi: '67',
			light: '223',
			mois: '2',
			token: '',
			
			// 设备下发命令状态
			autoIrrigation: false,   // 自动灌溉
			manualIrrigation: false, // 手动水泵
			autoLighting: false,     // 自动照明
			manualLighting: false,   // 手动照明
			
			// 工作状态
			ledStatus: 0,            
			pumpStatus: 0,
			
			// 🌟 新增：记录用户是否手动关闭了横幅
			isBannerDismissed: false
		};
	},
	computed: {
		hasDisconnectedSensor() {
			// 只要有任何一个数据等于 -1，就返回 true，触发报警
			return this.temp == -1 || this.humi == -1 || this.light == -1 || this.mois == -1;
		}
	},
	// 🌟 新增：监听器，用于传感器恢复后重置横幅状态
	watch: {
		hasDisconnectedSensor(newVal) {
			if (!newVal) {
				// 当所有的传感器都恢复正常后，重置关闭状态
				// 这样下次如果再断链，横幅还能弹出来
				this.isBannerDismissed = false;
			}
		}
	},
	onLoad() {
		const params = {
			product_id: 'Fv2Kuy99hL',
			version: '2022-05-01',
			access_key: 'YRv/6e09gRmjW02pHU2ZhYuZ4+ctW61JaZ1lLf/FEZc='
		};
		this.token = createCommonToken(params);
	},
	onShow() {
		this.fetchDevData();
		this.timer = setInterval(() => {
			this.fetchDevData();
		}, 3000);
	},
	onHide() {
		if(this.timer) {
			clearInterval(this.timer);
			this.timer = null;
		}
	},
	methods: {
		// 🌟 新增：关闭横幅的方法
		dismissBanner() {
			this.isBannerDismissed = true;
		},
		
		fetchDevData() {
			uni.request({
				url: 'https://iot-api.heclouds.com/thingmodel/query-device-property',
				method: 'GET',
				data: {
					product_id : 'Fv2Kuy99hL',
					device_name : 'ESP8266'
				},
				header: {
					'authorization': 'version=2018-10-31&res=products%2FFv2Kuy99hL%2Fdevices%2FESP8266&et=1810224000&method=md5&sign=8sZ2vjBOEPqvO2aNbYTLnQ%3D%3D'
				},
				success: (res) => {
					if(res.data && res.data.data) {
						this.ledStatus = res.data.data[3].value;  
						this.pumpStatus = res.data.data[4].value; 

						this.temp = res.data.data[5].value;      
						this.humi = res.data.data[7].value;      
						this.light = res.data.data[8].value;     
						this.mois = res.data.data[9].value;      

						res.data.data.forEach(item => {
							switch(item.id) {
								case 'Auto_water_mode': this.autoIrrigation = item.value; break;
								case 'Water_status':    this.manualIrrigation = item.value; break;
								case 'Auto_light_mode': this.autoLighting = item.value; break;
								case 'Light_status':    this.manualLighting = item.value; break;
							}
						});
					}
				}
			});
		},
		
		sendPropertyToCloud(paramsData) {
			uni.request({
				url: 'https://iot-api.heclouds.com/thingmodel/set-device-property',
				method: 'POST',
				data: {
					product_id : 'Fv2Kuy99hL',
					device_name : 'ESP8266',
					params: paramsData
				},
				header: {
					'authorization': 'version=2018-10-31&res=products%2FFv2Kuy99hL%2Fdevices%2FESP8266&et=1810224000&method=md5&sign=8sZ2vjBOEPqvO2aNbYTLnQ%3D%3D'
				},
				success: (res) => {
					console.log('云端同步控制成功:', paramsData, res.data);
				}
			});
		},

		switchAutoIrrigation(e) {
			this.autoIrrigation = e.detail.value;
			if (!this.autoIrrigation) {
				this.manualIrrigation = false;
			}
			this.sendPropertyToCloud({ "Auto_water_mode": this.autoIrrigation });
		},
		switchManualIrrigation(e) {
			this.manualIrrigation = e.detail.value;
			this.sendPropertyToCloud({ "Water_status": this.manualIrrigation });
		},
		switchAutoLighting(e) {
			this.autoLighting = e.detail.value;
			if (!this.autoLighting) {
				this.manualLighting = false;
			}
			this.sendPropertyToCloud({ "Auto_light_mode": this.autoLighting });
		},
		switchManualLighting(e) {
			this.manualLighting = e.detail.value;
			this.sendPropertyToCloud({ "Light_status": this.manualLighting });
		}
	}
};
</script>

<style lang="scss">
page {
	background-color: #f4f6f9;
}

.page-wrapper {
	width: 100%;
}

/* 🌟 修改：优化横幅布局，靠右显示关闭按钮 */
.warning-banner {
	background-color: #fff0f0;
	padding: 24rpx 30rpx;
	display: flex;
	align-items: center;
	justify-content: space-between; /* 从 center 改为两端对齐 */
	box-shadow: 0 4rpx 10rpx rgba(255, 59, 48, 0.1);
	border-bottom: 2rpx solid #ffdcdb;
	width: 100%;
	box-sizing: border-box;

	.warning-content {
		display: flex;
		align-items: center;
	}

	.warning-icon {
		font-size: 32rpx;
		margin-right: 12rpx;
	}

	.warning-text {
		font-size: 28rpx;
		color: #ff3b30;
		font-weight: bold;
	}
	
	.close-icon {
		font-size: 36rpx;
		color: #ffaa99;
		padding: 0 10rpx;
		font-weight: bold;
	}
}

.container {
	padding: 30rpx;
	padding-bottom: 60rpx;
}

.header {
	margin-bottom: 40rpx;
	padding-top: 10rpx;
	.title {
		font-size: 48rpx;
		font-weight: bold;
		color: #333;
		display: block;
	}
	.subtitle {
		font-size: 28rpx;
		color: #888;
		margin-top: 10rpx;
		display: block;
	}
}

.card {
	background-color: #ffffff;
	border-radius: 24rpx;
	padding: 30rpx;
	margin-bottom: 30rpx;
	box-shadow: 0 4rpx 16rpx rgba(0, 0, 0, 0.04);
}

.card-header {
	margin-bottom: 30rpx;
	.card-title {
		font-size: 32rpx;
		font-weight: 600;
		color: #333;
	}
}

/* 传感器网格 */
.data-grid {
	display: flex;
	flex-wrap: wrap;
	justify-content: space-between;
}

.data-item {
	width: 48%;
	background-color: #f8f9fa;
	border-radius: 16rpx;
	padding: 24rpx;
	margin-bottom: 20rpx;
	box-sizing: border-box;
	display: flex;
	flex-direction: column;
	align-items: center;
	.icon { font-size: 40rpx; margin-bottom: 12rpx; }
	.value {
		font-size: 40rpx;
		font-weight: bold;
		color: #2c3e50;
		margin-bottom: 6rpx;
		.unit { font-size: 24rpx; font-weight: normal; color: #7f8c8d; margin-left: 4rpx; }
	}
	.disconnect-text {
		font-size: 32rpx;
		font-weight: bold;
		color: #ff3b30; 
		margin-bottom: 6rpx;
		line-height: 1.2;
	}
	.label { font-size: 24rpx; color: #999; }
}

/* 可视化状态样式 */
.status-container {
	display: flex;
	justify-content: space-around;
	align-items: center;
	padding: 10rpx 0;
}

.status-item {
	display: flex;
	flex-direction: column;
	align-items: center;
	width: 45%;
	
	.status-label {
		font-size: 26rpx;
		color: #666;
		margin-bottom: 16rpx;
	}
	
	.status-badge {
		display: flex;
		align-items: center;
		background-color: #f0f0f2;
		color: #7f8c8d;
		padding: 10rpx 24rpx;
		border-radius: 30rpx;
		font-size: 26rpx;
		font-weight: bold;
		transition: all 0.3s ease;
		
		.badge-dot {
			width: 12rpx;
			height: 12rpx;
			background-color: #95a5a6;
			border-radius: 50%;
			margin-right: 12rpx;
		}
		
		&.active {
			background-color: #e8f8f5;
			color: #2ecc71;
			
			.badge-dot {
				background-color: #2ecc71;
				box-shadow: 0 0 10rpx #2ecc71;
			}
		}
	}
}

.divider-v {
	width: 1px;
	height: 80rpx;
	background-color: #eef0f2;
}

/* 控制列表 */
.control-list {
	.control-item {
		display: flex;
		justify-content: space-between;
		align-items: center;
		padding: 10rpx 0;
		.item-info {
			display: flex;
			flex-direction: column;
			.item-name {
				font-size: 30rpx;
				color: #333;
				&.disabled-text { color: #bbb; }
			}
			.item-desc { font-size: 22rpx; color: #999; margin-top: 6rpx; }
		}
	}
	.divider {
		height: 1px;
		background-color: #eee;
		margin: 20rpx 0;
	}
}
</style>