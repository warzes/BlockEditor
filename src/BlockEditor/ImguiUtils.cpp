#include "stdafx.h"
#include "ImguiUtils.h"

bool rlImGuiImageButton(const char* name, const RLTexture* image)
{
	return ImGui::ImageButton(name, (ImTextureID)image, ImVec2(float(image->width), float(image->height)));
}