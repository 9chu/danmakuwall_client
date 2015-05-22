# 弹幕墙 接收端

本项目旨在实现实时的弹幕吐槽功能，这是项目的前端部分。

项目仅限Windows平台运行，基于fancy2d进行渲染然后拷贝显示到屏幕上。

## 编译

项目需要在Visual Stuido 2013或更高版本进行编译。

NOTE: 需要先使用同版本Visual Stuido编译**[fancy2d](http://github.com/9chu/fancy2d)**项目。

## 部署配置

### 发布时目录结构

	\
	..danmakuwall_client.exe
	..fancy2D.dll
	..config.json
	..D3DX9_43.dll(可选)

### 配置文件

拷贝```config.default.json```并重命名为```config.json```

#### 配置项含义

- *fetchApiUrl*: server端所用的fetchApi地址（不支持HTTPS）
- *fetchAuthKey*: fetch所用key
- *fetchTimeout*: 拉取弹幕数据时的最大超时时间
- *reconnectTime*: 从server端丢失连接重新连接的时间
- *reconnectInfo*: 重新连接的提示弹幕
- *autoTopMostTime*: 自动置顶时钟
- *danmakuAlpha*: 弹幕层的整体透明度(50~255)
- *styles.font*: 字体名称，请从注册表的Fonts项中选择合适字体
- *styles.margin*: 弹幕的间距（像素）
- *styles.innerMargin*: 一条弹幕内的行间距（像素）
- *styles.screenPadding*: 屏幕上下留空距离（像素）
- *styles.fontSize*: 指定各种字体型号所用的大小（像素）
- *danmaku.top.lifetime*: 顶端弹幕的存活时间
- *danmaku.bottom.lifetime*: 底端弹幕的存活时间
- *danmaku.classical.velocityFactory*: 横向弹幕漂浮速度在弹幕宽度上的因子
- *danmaku.classical.baseVelocity*: 横向弹幕漂浮的基本速度
- *danmaku.classical.positionResetTime*: 横向弹幕出生位置归零的计数器
- *trayIcon.title*: 托盘图标的显示名称
- *trayIcon.menuStrip.clear*: 托盘菜单"清空弹幕池"显示文本
- *trayIcon.menuStrip.exit*: 托盘菜单"退出"显示文本

## 已知缺陷

- 由于弹幕使用Direct3D渲染，对显卡有一定要求
- 画面渲染后将从显存拷贝到内存而后拷贝到屏幕上，推荐在低分辨率下运行

## TO DO LIST

- 解决托盘图标右键菜单无法响应焦点丢失事件
- 解决分辨率切换时显示的自适应
- 解决explorer崩溃后托盘图标重置的问题

## 许可

项目代码基于**MIT LICENSE**许可

