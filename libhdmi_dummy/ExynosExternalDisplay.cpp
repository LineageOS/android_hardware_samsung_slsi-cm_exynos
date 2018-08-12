#include "ExynosMPPModule.h"
#include "ExynosExternalDisplay.h"

ExynosExternalDisplay::ExynosExternalDisplay(struct exynos5_hwc_composer_device_1_t *pdev) :
    ExynosDisplay(1)
{
    this->mHwc = pdev;
    mMPPs[0] = new ExynosMPPModule(this, HDMI_GSC_IDX);
    mEnabled = false;
    mBlanked = false;
    mUseSubtitles = false;

}

ExynosExternalDisplay::~ExynosExternalDisplay()
{
    delete mMPPs[0];
}

int ExynosExternalDisplay::prepare(hwc_display_contents_1_t *contents __unused)
{
    return 0;
}

int ExynosExternalDisplay::set(hwc_display_contents_1_t *contents __unused)
{
    return 0;
}

int ExynosExternalDisplay::openHdmi()
{
    return 0;
}

void ExynosExternalDisplay::setHdmiStatus(bool status __unused)
{
}

bool ExynosExternalDisplay::isPresetSupported(unsigned int preset __unused)
{
    return false;
}

int ExynosExternalDisplay::getConfig()
{
    return 0;
}

int ExynosExternalDisplay::enableLayer(hdmi_layer_t &hl __unused)
{
    return 0;
}

void ExynosExternalDisplay::disableLayer(hdmi_layer_t &hl __unused)
{
}

int ExynosExternalDisplay::enable()
{
    return 0;
}

void ExynosExternalDisplay::disable()
{
}

int ExynosExternalDisplay::output(hdmi_layer_t &hl __unused, hwc_layer_1_t &layer __unused, private_handle_t *h __unused, int acquireFenceFd __unused, int *releaseFenceFd __unused)
{
    return 0;
}

void ExynosExternalDisplay::skipStaticLayers(hwc_display_contents_1_t *contents __unused, int ovly_idx __unused)
{
}

void ExynosExternalDisplay::setPreset(int preset __unused)
{
}

int ExynosExternalDisplay::convert3DTo2D(int preset __unused)
{
    return 0;
}

void ExynosExternalDisplay::calculateDstRect(int src_w __unused, int src_h __unused, int dst_w __unused, int dst_h __unused, struct v4l2_rect *dst_rect __unused)
{
}

void ExynosExternalDisplay::setHdcpStatus(int status __unused)
{
}

void ExynosExternalDisplay::setAudioChannel(uint32_t channels __unused)
{
}

uint32_t ExynosExternalDisplay::getAudioChannel()
{
    return 0;
}

int ExynosExternalDisplay::blank()
{
    return 0;
}
