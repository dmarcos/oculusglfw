#include <iostream>
#include <algorithm>

#include <glad/include/glad.h>
#include <GLFW/glfw3.h>
#include <LibOVR/OVR_CAPI_GL.h>
#include <LibOVR/Extras/OVR_Math.h>

#ifndef _WINDEF_
typedef unsigned long DWORD;
#endif

typedef struct SwapChain {
  ovrTextureSwapChain ColorTextureChain;
  ovrTextureSwapChain DepthTextureChain;
  ovrSizei textureSize;
  GLuint fboId;
} EyeSwapChain;

// enable optimus!
extern "C" {
  _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

int main(){

  EyeSwapChain eyes[2];

  // Oculus: Initialize
  ovrResult result = ovr_Initialize(nullptr);
  if (OVR_FAILURE(result)) {
  }

  long long frameIndex = 0;
  ovrSession session;
  ovrGraphicsLuid luid;
  result = ovr_Create(&session, &luid);
  if (OVR_FAILURE(result))
  {
    ovr_Shutdown();
  }

  ovrHmdDesc hmdDesc = ovr_GetHmdDesc(session);
  if (hmdDesc.Type != ovrHmd_None) {
    std::cout << "HEADSET CONNECTED" << std::endl;
  } else {
    std::cout << "HEADSET NOT CONNECTED" << std::endl;
  }

  // End Oculus: Initialize

  if (!glfwInit()) {
    std::cout << "glfw Initialization failed" << std::endl;
  }

  GLFWwindow* window = glfwCreateWindow(640, 480, "My Title", NULL, NULL);
  if (!window) {
    glfwTerminate();
    std::cout << "glfw window initialization failed" << std::endl;
  }

  glfwMakeContextCurrent(window);
  gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
  glfwSwapInterval(1);

  // Oculus: Initialize Swap Chain

  ovrSizei leftEyeTextureSize = ovr_GetFovTextureSize(session, ovrEye_Left, hmdDesc.DefaultEyeFov[ovrEye_Left], 1);
  ovrSizei rightEyeTextureSize = ovr_GetFovTextureSize(session, ovrEye_Right, hmdDesc.DefaultEyeFov[ovrEye_Right], 1);
  eyes[0].textureSize.w = leftEyeTextureSize.w;
  eyes[0].textureSize.h = leftEyeTextureSize.h;
  eyes[1].textureSize.w = rightEyeTextureSize.w;
  eyes[1].textureSize.h = rightEyeTextureSize.h;

  ovrSizei bufferSize;
  bufferSize.w  = leftEyeTextureSize.w + rightEyeTextureSize.w;
  bufferSize.h = std::max(leftEyeTextureSize.h, rightEyeTextureSize.h);

  // Make eye render buffers
  for (int eye = 0; eye < 2; ++eye) {
    ovrTextureSwapChainDesc desc = {};
    desc.Type = ovrTexture_2D;
    desc.ArraySize = 1;
    desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
    desc.Width = bufferSize.w;
    desc.Height = bufferSize.h;
    desc.MipLevels = 1;
    desc.SampleCount = 1;
    desc.StaticImage = ovrFalse;

    result = ovr_CreateTextureSwapChainGL(session, &desc, &eyes[eye].ColorTextureChain);
    int length = 0;
    ovr_GetTextureSwapChainLength(session, eyes[eye].ColorTextureChain, &length);

    if (!OVR_SUCCESS(result)) {
      std::cout << "SWAP CHAIN ERROR " << result << std::endl;
    } else {
      std::cout << "SWAP CHAIN CREATED SUCCESS" << std::endl;
      for (int i = 0; i < length; ++i) {
        GLuint textureId;
        ovr_GetTextureSwapChainBufferGL(session, eyes[eye].ColorTextureChain, i, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      }
    }

    desc.Format = OVR_FORMAT_D32_FLOAT;

    result = ovr_CreateTextureSwapChainGL(session, &desc, &eyes[eye].DepthTextureChain);
    ovr_GetTextureSwapChainLength(session, eyes[eye].DepthTextureChain, &length);

    if (!OVR_SUCCESS(result)) {
      std::cout << "SWAP CHAIN ERROR " << result << std::endl;
    } else {
      std::cout << "SWAP CHAIN CREATED SUCCESS" << std::endl;
      for (int i = 0; i < length; ++i) {
        GLuint textureId;
        ovr_GetTextureSwapChainBufferGL(session, eyes[eye].DepthTextureChain, i, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      }
    }

    glGenFramebuffers(1, &eyes[eye].fboId);

  }

  // End Oculus: Initialize Swap Chain

  glClearColor(1.0, 0.0, 1.0, 1.0);

  while (!glfwWindowShouldClose(window)) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    // Eye textures clear.
    GLuint colorTextureId;
    GLuint depthTextureId;

    // Call ovr_GetRenderDesc each frame to get the ovrEyeRenderDesc, as the returned values (e.g. HmdToEyePose) may change at runtime.
    ovrEyeRenderDesc eyeRenderDesc[2];
    eyeRenderDesc[0] = ovr_GetRenderDesc(session, ovrEye_Left, hmdDesc.DefaultEyeFov[0]);
    eyeRenderDesc[1] = ovr_GetRenderDesc(session, ovrEye_Right, hmdDesc.DefaultEyeFov[1]);

    // Get eye poses, feeding in correct IPD offset
    ovrPosef EyeRenderPose[2];
    ovrPosef HmdToEyePose[2] = {
      eyeRenderDesc[0].HmdToEyePose,
      eyeRenderDesc[1].HmdToEyePose
    };

    double sensorSampleTime;    // sensorSampleTime is fed into the layer later
    ovr_GetEyePoses(session, frameIndex, ovrTrue, HmdToEyePose, EyeRenderPose, &sensorSampleTime);

    ovrTimewarpProjectionDesc posTimewarpProjectionDesc = {};

    for (int eye = 0; eye < 2; ++eye) {

      {
        int curIndex;
        ovr_GetTextureSwapChainCurrentIndex(session, eyes[eye].ColorTextureChain, &curIndex);
        ovr_GetTextureSwapChainBufferGL(session, eyes[eye].ColorTextureChain, curIndex, &colorTextureId);
      }
      {
        int curIndex;
        ovr_GetTextureSwapChainCurrentIndex(session, eyes[eye].DepthTextureChain, &curIndex);
        ovr_GetTextureSwapChainBufferGL(session, eyes[eye].DepthTextureChain, curIndex, &depthTextureId);
      }

      glBindFramebuffer(GL_FRAMEBUFFER, eyes[eye].fboId);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTextureId, 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTextureId, 0);

      glViewport(0, 0, eyes[eye].textureSize.w, eyes[eye].textureSize.h);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glEnable(GL_FRAMEBUFFER_SRGB);

      ovr_CommitTextureSwapChain(session, eyes[eye].ColorTextureChain);
      ovr_CommitTextureSwapChain(session, eyes[eye].DepthTextureChain);

      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
    }

    // Distortion, Present and flush/sync
    ovrLayerEyeFovDepth ld = {};
    ld.Header.Type  = ovrLayerType_EyeFovDepth;
    ld.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;   // Because OpenGL.
    ld.ProjectionDesc = posTimewarpProjectionDesc;

    for (int eye = 0; eye < 2; ++eye)
    {
      ld.ColorTexture[eye] = eyes[eye].ColorTextureChain;
      ld.DepthTexture[eye] = eyes[eye].DepthTextureChain;
      ld.Viewport[eye]     = OVR::Recti(eyes[eye].textureSize);
      ld.Fov[eye]          = hmdDesc.DefaultEyeFov[eye];
      ld.RenderPose[eye]   = EyeRenderPose[eye];
      ld.SensorSampleTime  = sensorSampleTime;
    }

    ovrLayerHeader* layers = &ld.Header;
    result = ovr_SubmitFrame(session, frameIndex, nullptr, &layers, 1);

    frameIndex++;
    // End of Distortion, Present and flush/sync

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return 1;
}
