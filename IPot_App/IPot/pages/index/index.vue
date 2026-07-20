<template>
	<view class="container">
		<view class="header">
			<text class="main-title">嵌入粽师</text>
			<text class="sub-title">让嵌入式开发更有趣味</text>
		</view>

		<view class="column-grid">
			<view class="column-item active" @tap="navigateToIPot">
				<view class="icon-box">
					<image src="/static/logo.png" mode="aspectFit" class="icon"></image>
				</view>
				<view class="info">
					<text class="item-name">IPot智慧盆栽</text>
					<text class="item-desc">智能环境监测与植物养护</text>
				</view>
				<view class="status-tag">进入项目</view>
			</view>

			<view class="column-item disabled" @tap="showTip">
				<view class="icon-box gray">
					<text class="placeholder-icon">?</text>
				</view>
				<view class="info">
					<text class="item-name">IGlass</text>
					<text class="item-desc">项目策划中</text>
				</view>
				<view class="status-tag gray">开发中...</view>
			</view>

			<view class="column-item disabled" @tap="showTip">
				<view class="icon-box gray">
					<text class="placeholder-icon">?</text>
				</view>
				<view class="info">
					<text class="item-name">端侧AI实验室</text>
					<text class="item-desc">敬请期待</text>
				</view>
				<view class="status-tag gray">待开发</view>
			</view>
		</view>
		
		<view class="footer">
			<text>© 2026 嵌入粽师团队</text>
		</view>
	</view>
</template>

<script>
export default {
	data() {
		return {};
	},
	methods: {
		// 跳转到 IPot 页面
		navigateToIPot() {
			uni.navigateTo({
				// 注意：请确保你的页面路径与 pages.json 中配置的一致
				url: '/pages/IPot/IPot' 
			});
		},
		// 点击未开发栏目的提示
		showTip() {
			uni.showToast({
				title: '该功能正在开发中...',
				icon: 'none'
			});
		}
	}
};
</script>

<style lang="scss">
.container {
	min-height: 100vh;
	background-color: #f5f7f9;
	padding: 40rpx;
	display: flex;
	flex-direction: column;
}

.header {
	margin-top: 60rpx;
	margin-bottom: 80rpx;
	text-align: center;
	
	.main-title {
		font-size: 64rpx;
		font-weight: bold;
		color: #2c3e50;
		display: block;
		letter-spacing: 4rpx;
	}
	
	.sub-title {
		font-size: 28rpx;
		color: #7f8c8d;
		margin-top: 20rpx;
		display: block;
	}
}

.column-grid {
	flex: 1;
}

.column-item {
	background-color: #ffffff;
	border-radius: 24rpx;
	padding: 30rpx;
	margin-bottom: 40rpx;
	display: flex;
	align-items: center;
	box-shadow: 0 4rpx 16rpx rgba(0, 0, 0, 0.05);
	transition: transform 0.2s;
	position: relative;

	&:active {
		transform: scale(0.98);
	}

	&.disabled {
		opacity: 0.7;
	}

	.icon-box {
		width: 100rpx;
		height: 100rpx;
		background-color: #e8f5e9;
		border-radius: 18rpx;
		display: flex;
		align-items: center;
		justify-content: center;
		margin-right: 30rpx;
		
		&.gray {
			background-color: #eee;
		}

		.icon {
			width: 60rpx;
			height: 60rpx;
		}
		
		.placeholder-icon {
			font-size: 40rpx;
			color: #999;
		}
	}

	.info {
		flex: 1;
		
		.item-name {
			font-size: 34rpx;
			font-weight: 600;
			color: #333;
			display: block;
		}
		
		.item-desc {
			font-size: 24rpx;
			color: #999;
			margin-top: 8rpx;
			display: block;
		}
	}

	.status-tag {
		font-size: 22rpx;
		padding: 6rpx 16rpx;
		background-color: #4caf50;
		color: #fff;
		border-radius: 10rpx;
		
		&.gray {
			background-color: #bbb;
		}
	}
}

.footer {
	text-align: center;
	padding: 40rpx 0;
	text-align: center;
	
	text {
		font-size: 24rpx;
		color: #bdc3c7;
	}
}
</style>