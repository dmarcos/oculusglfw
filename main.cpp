#include <iostream>
#include <algorithm>

#include <glad/include/glad.h>
#include <GLFW/glfw3.h>
#include <LibOVR/OVR_CAPI_GL.h>

#ifndef _WINDEF_
typedef unsigned long DWORD;
#endif

// enable optimus!
extern "C" {
  _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

int main(){

  GLuint fboId;
  GLuint eyeTextures[] = {0, 0};

  // Oculus: Initialize
  ovrResult result = ovr_Initialize(nullptr);
  if (OVR_FAILURE(result)) {
  }

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

  ovrSizei recommenedTex0Size = ovr_GetFovTextureSize(session, ovrEye_Left, hmdDesc.DefaultEyeFov[ovrEye_Left], 1);
  ovrSizei recommenedTex1Size = ovr_GetFovTextureSize(session, ovrEye_Right, hmdDesc.DefaultEyeFov[ovrEye_Right], 1);
  ovrSizei bufferSize;
  bufferSize.w  = recommenedTex0Size.w + recommenedTex1Size.w;
  bufferSize.h = std::max(recommenedTex0Size.h, recommenedTex1Size.h);

  ovrTextureSwapChain textureSwapChain;

  ovrTextureSwapChainDesc desc = {};
  desc.Type = ovrTexture_2D;
  desc.ArraySize = 1;
  desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
  desc.Width = bufferSize.w;
  desc.Height = bufferSize.h;
  desc.MipLevels = 1;
  desc.SampleCount = 1;
  desc.StaticImage = ovrFalse;

  result = ovr_CreateTextureSwapChainGL(session, &desc, &textureSwapChain);
  int length = 0;
  ovr_GetTextureSwapChainLength(session, textureSwapChain, &length);
  std::cout << "XXXXXXXXXXXXXXXX " << length << std::endl;
  if (!OVR_SUCCESS(result)) {
    std::cout << "SWAP CHAIN ERROR " << result << std::endl;
  } else {
    std::cout << "SWAP CHAIN CREATED SUCCESS" << std::endl;
    for (int i = 0; i < length; ++i) {
       ovr_GetTextureSwapChainBufferGL(session, textureSwapChain, i, &eyeTextures[i]);
       glBindTexture(GL_TEXTURE_2D, eyeTextures[i]);

       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    glGenFramebuffers(1, &fboId);
  }

  // End Oculus: Initialize Swap Chain

  glClearColor(1.0, 0.0, 1.0, 1.0);

  while (!glfwWindowShouldClose(window)) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, fboId);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, eyeTextures[0], 0);

    glViewport(0, 0, recommenedTex0Size.w, recommenedTex0Size.h);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_FRAMEBUFFER_SRGB);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, eyeTextures[1], 0);

    glViewport(0, 0, recommenedTex1Size.w, recommenedTex1Size.h);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_FRAMEBUFFER_SRGB);

    ovr_CommitTextureSwapChain(session, textureSwapChain);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return 1;
}
