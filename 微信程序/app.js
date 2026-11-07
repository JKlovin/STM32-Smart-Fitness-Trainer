// app.js
App({
  onLaunch() {
    wx.requirePrivacyAuthorize({
      success: () => console.log('隐私协议授权成功'),
      fail: err => console.error('隐私授权失败:', err)
    })
  }
})