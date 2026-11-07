import * as echarts from '../../ec-canvas/echarts'; // 确保路径正确

Page({
  data: {
    ecPitch: { onInit: null },
    pitchChart: null,
    devices: [],
    showDeviceList: false,
    scanning: false,
    connected: false,
    currentDeviceName: '',
    activeDevice: '',
    pitchData: [],
    maxDataPoints: 20,
    lastUpdateTime: null,
    currentVal: null,  // 新增字段用于存储实时数据
    currentGravity: null, // 新增字段用于存储重力数据
  },

  initChart: function(canvas, width, height) {
    console.log('初始化 pitch 图表');
    if (!canvas) {
      console.error('无效的 canvas 对象');
      return;
    }

    const chart = echarts.init(canvas, null, { width, height });
    canvas.setChart(chart);
    this.pitchChart = chart;

    const option = {
      xAxis: { 
        type: 'category', 
        data: [], 
        boundaryGap: false 
      },
      yAxis: {
        type: 'value',
        min: -300, // 设置纵轴范围
        max: 1500,
        axisLine: { show: false },
        splitLine: { lineStyle: { type: 'dashed' } },
        axisLabel: {
          fontSize: 8,  // 修改纵轴字体大小
          color: '#333'  // 可选，修改字体颜色
        }
      },
      series: [{
        type: 'line',
        smooth: true,
        symbol: 'circle',
        symbolSize: 2,
        lineStyle: { width: 2 },
        itemStyle: { color: '#409EFF' },
        data: []
      }],
      grid: { top: 30, left: 40, right: 20, bottom: 30 }
    };

    chart.setOption(option);
    return chart;
  },

  onLoad() {
    this.setData({
      ecPitch: {
        onInit: (canvas, width, height) => this.initChart(canvas, width, height)
      }
    });

    wx.openBluetoothAdapter({
      success: () => {
        this.getBluetoothAdapterState();
      },
      fail: (res) => {
        console.error('蓝牙适配器打开失败:', res);
        wx.showToast({ title: '请确保蓝牙已开启', icon: 'none' });
      }
    });
  },

  getBluetoothAdapterState() {
    wx.getBluetoothAdapterState({
      success: (res) => {
        if (res.available) {
          console.log('蓝牙适配器可用');
          this.startBluetoothDevicesDiscovery();  // 蓝牙适配器可用，开始扫描设备
        } else {
          console.log('蓝牙适配器不可用');
          wx.showToast({ title: '蓝牙适配器不可用', icon: 'none' });
        }
      },
      fail: (err) => {
        console.error('获取蓝牙适配器状态失败:', err);
        wx.showToast({ title: '获取蓝牙适配器状态失败', icon: 'none' });
      }
    });
  },


startBluetoothDevicesDiscovery() {
  if (this.data.scanning) return;
  this.setData({ devices: [], scanning: true, showDeviceList: true });

  wx.startBluetoothDevicesDiscovery({
    allowDuplicatesKey: false,
    success: () => {
      wx.onBluetoothDeviceFound((res) => {
        res.devices.forEach(device => {
          if (device.name && !this.data.devices.find(d => d.deviceId === device.deviceId)) {
            this.setData({ devices: this.data.devices.concat(device) });
          }
        });
      });
      setTimeout(() => {
        wx.stopBluetoothDevicesDiscovery();
        this.setData({ scanning: false });
      }, 10000);
    },
    fail: (err) => {
      console.error('开始搜索设备失败:', err);
      wx.showToast({ title: '搜索设备失败', icon: 'none' });
    }
  });
},

  connectDevice(e) {
    const deviceId = e.currentTarget.dataset.id;
    const device = this.data.devices.find(d => d.deviceId === deviceId);

    wx.createBLEConnection({
      deviceId,
      success: () => {
        this.setData({ connected: true, currentDeviceName: device.name, activeDevice: deviceId });
        wx.showToast({ title: '连接成功', icon: 'success' });
        this.getBLEServices(deviceId);
        this.closeDeviceList();
      },
      fail: (err) => {
        wx.showToast({ title: '连接失败', icon: 'none' });
        console.error('连接设备失败:', err);
      }
    });
  },

  getBLEServices(deviceId) {
    wx.getBLEDeviceServices({
      deviceId,
      success: (res) => {
        console.log('设备服务:', res.services);
        // 遍历设备的所有服务，不再指定特定的服务 UUID
        res.services.forEach(service => {
          this.getBLECharacteristics(deviceId, service.uuid);
        });
      },
      fail: (err) => {
        console.error('获取服务失败:', err);
      }
    });
  },

  getBLECharacteristics(deviceId, serviceId) {
    wx.getBLEDeviceCharacteristics({
      deviceId,
      serviceId,
      success: (res) => {
        console.log('设备特征:', res.characteristics);
        // 遍历设备的所有特征，不再指定特定的特征 UUID
        res.characteristics.forEach(item => {
          if (item.properties.notify) {
            wx.notifyBLECharacteristicValueChange({
              deviceId,
              serviceId,
              characteristicId: item.uuid,
              state: true,
              success: () => {
                wx.onBLECharacteristicValueChange((characteristic) => {
                  this.processData(characteristic.value);
                });
              },
              fail: (err) => {
                console.error('开启通知失败:', err);
              }
            });
          }
        });
      },
      fail: (err) => {
        console.error('获取特征失败:', err);
      }
    });
  },

  processData(value) {
    const decodedData = this.decodeBase64(value);
    console.log('解码后的数据:', decodedData);
  
    const match = decodedData.match(/val:([-\d.]+)/);
    if (match) {
      const val = parseFloat(match[1]);
      console.log(`接收到数据 - val: ${val}`);
    
      // 计算重力数据，将 val 除以 10
      const gravity = (val / 10).toFixed(2);
      console.log(`计算得到的重力数据: ${gravity}`);

      // 更新图表数据
      this.updateChart('pitch', val);  // 只更新 pitch 图表
  
      // 更新 currentVal 和 currentGravity
      this.setData({
        currentVal: val,      // 将当前的 val 数据存储在 currentVal 中
        currentGravity: gravity  // 将计算出的重力数据存储在 currentGravity 中
      });
    } else {
      console.warn('数据格式不匹配:', decodedData);
    }
  },

  decodeBase64(encodedData) {
    const base64String = wx.arrayBufferToBase64(encodedData);
    return this.base64Decode(base64String);
  },

  base64Decode(str) {
    const chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=';
    let output = '';
    let buffer = 0;
    let bitsCollected = 0;

    for (let char of str) {
      const charIndex = chars.indexOf(char);
      if (charIndex === -1) continue; // 忽略无效字符

      buffer = (buffer << 6) | charIndex;
      bitsCollected += 6;

      if (bitsCollected >= 8) {
        bitsCollected -= 8;
        output += String.fromCharCode((buffer >> bitsCollected) & 0xFF);
      }
    }
    return output;
  },

  updateChart(type, value) {
    console.log(`更新 ${type} 图表数据`);
    const dataKey = `${type}Data`;
    const chart = this.pitchChart;  // 只处理 pitch 图表
  
    if (!chart) {
      console.error(`${type} 图表未初始化`);
      return;
    }
  
    // 创建新的数据点数组，只保留最新的几个数据点
    const newData = [...this.data[dataKey], value];
  
    // 限制数据点的数量（例如，最多保留 50 个数据点）
    if (newData.length > this.data.maxDataPoints) {
      newData.shift();  // 删除最旧的数据点（第一个）
    }
  
    // 显示新的数据点
    const xData = newData.map((_, index) => index + 1);
  
    // 更新数据
    this.setData({ [dataKey]: newData });
  
    // 实时更新图表
    chart.setOption({
      xAxis: { data: xData },
      series: [{ data: newData }]
    });
  
    // 更新最后更新时间
    this.setData({ lastUpdateTime: Date.now() });
  
    console.log(`当前 val 数据: ${JSON.stringify(newData)}`);
  },  

  disconnect() {
    wx.closeBLEConnection({
      deviceId: this.data.activeDevice,
      success: () => {
        this.resetConnection();
        wx.showToast({ title: '已断开连接', icon: 'success' });
  
        // 在断开连接后，确保重新启动蓝牙设备搜索
        this.startBluetoothDevicesDiscovery();  // 确保蓝牙设备搜索重新启动
      },
      fail: (err) => {
        console.error('断开连接失败:', err);
        wx.showToast({ title: '断开连接失败', icon: 'none' });
      }
    });
  },


  resetConnection() {
    this.setData({
      connected: false,
      activeDevice: '',
      currentDeviceName: '',
      pitchData: []
    });
  },

  closeDeviceList() {
    this.setData({
      showDeviceList: false,
      scanning: false
    });
  }
});
