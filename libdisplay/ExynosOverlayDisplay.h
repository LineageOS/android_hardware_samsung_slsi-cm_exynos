#ifndef EXYNOS_PRIMARY_DISPLAY_H
#define EXYNOS_PRIMARY_DISPLAY_H

#include "ExynosHWC.h"
#include "ExynosDisplay.h"

#ifndef V4L2_DV_1080P60
#define V4L2_DV_1080P60 18
#endif

#define S3D_ERROR -1
#define HDMI_PRESET_DEFAULT V4L2_DV_1080P60
#define HDMI_PRESET_ERROR -1

class ExynosMPPModule;

class ExynosOverlayDisplay : public ExynosDisplay {
    public:
        /* Methods */
        ExynosOverlayDisplay(int numGSCs, struct exynos5_hwc_composer_device_1_t *pdev);
        ~ExynosOverlayDisplay();

        virtual int prepare(hwc_display_contents_1_t *contents);
        virtual int set(hwc_display_contents_1_t *contents);
        virtual void freeMPP();
        virtual void handleTotalBandwidthOverload(hwc_display_contents_1_t *contents);

        int getCompModeSwitch();
        int32_t getDisplayAttributes(const uint32_t attribute);

        /* Fields */
        ExynosMPPModule         **mMPPs;

        exynos5_hwc_post_data_t  mPostData;
        const private_module_t   *mGrallocModule;

        fb_win_config mLastConfig[NUM_HW_WINDOWS];
        size_t                   mLastFbWindow;
        const void               *mLastHandles[NUM_HW_WINDOWS];
        exynos5_gsc_map_t        mLastGscMap[NUM_HW_WINDOWS];
        const void               *mLastLayerHandles[NUM_VIRT_OVER];
        int                      mLastOverlayWindowIndex;
        int                      mLastOverlayLayerIndex;
        int                      mVirtualOverlayFlag;

        bool                     mForceFbYuvLayer;
        int                      mCountSameConfig;
        /* g3d = 0, gsc = 1 */
        int                      mConfigMode;
        video_layer_config       mPrevSrcConfig[MAX_VIDEO_LAYERS];
        video_layer_config       mPrevDstConfig[MAX_VIDEO_LAYERS];

        int                      mGscLayers;

        bool                     mPopupPlayYuvContents;
        bool                     mHasCropSurface;
        int                      mYuvLayers;

        bool                     mBypassSkipStaticLayer;
        uint32_t                 mDmaChannelMaxBandwidth[MAX_NUM_FIMD_DMA_CH];
        uint32_t                 mDmaChannelMaxOverlapCount[MAX_NUM_FIMD_DMA_CH];

        bool                     mGscUsed;
        int                      mCurrentGscIndex;
        bool                     mFbNeeded;
        size_t                   mFirstFb;
        size_t                   mLastFb;
        bool                     mForceFb;
        int                      mForceOverlayLayerIndex;
        bool                     mRetry;

    protected:
        /* Methods */
        void configureOtfWindow(hwc_rect_t &displayFrame,
                int32_t blending, int32_t planeAlpha, int format, fb_win_config &cfg);
        void configureHandle(private_handle_t *handle, hwc_frect_t &sourceCrop,
                hwc_rect_t &displayFrame, int32_t blending, int32_t planeAlpha, int fence_fd, fb_win_config &cfg,
                int32_t win_idx);
        int clearDisplay();
        void skipStaticLayers(hwc_display_contents_1_t *contents);
        void determineSupportedOverlays(hwc_display_contents_1_t *contents);
        void determineBandwidthSupport(hwc_display_contents_1_t *contents);
        void determineYuvOverlay(hwc_display_contents_1_t *contents);
        void assignWindows(hwc_display_contents_1_t *contents);
        bool assignGscLayer(hwc_layer_1_t &layer, int index, int nextWindow);
        int postGscOtf(hwc_layer_1_t &layer, fb_win_config *config, int win_map, int index);
        void handleStaticLayers(hwc_display_contents_1_t *contents, fb_win_config_data &win_data, int tot_ovly_wins);
        void cleanupGscs();

        virtual int postGscM2M(hwc_layer_1_t &layer, fb_win_config *config, int win_map, int index);
        virtual void forceYuvLayersToFb(hwc_display_contents_1_t *contents);
        virtual void configureOverlay(hwc_layer_1_t *layer, fb_win_config &cfg, int32_t win_idx);
        virtual bool isOverlaySupported(hwc_layer_1_t &layer, size_t i);
        virtual int postFrame(hwc_display_contents_1_t *contents);
        virtual int waitForRenderFinish(buffer_handle_t *handle, int buffers);
        virtual void handleOffscreenRendering(hwc_layer_1_t &layer);
        virtual int getMPPForUHD(hwc_layer_1_t &layer);
};

#endif
