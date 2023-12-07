#include "stdafx.h"
#include "PickMode.h"
#include "RLMath.h"
#include "EditorApp.h"
#include "ImguiUtils.h"
#include "Core.h"

#define FRAME_SIZE 196
#define ICON_SIZE 64

PickMode::Frame::Frame()
    : Frame(std::filesystem::path(), std::filesystem::path())
{}

PickMode::Frame::Frame(const std::filesystem::path filePath, const std::filesystem::path rootDir)
{
    this->filePath = filePath;
    label = std::filesystem::relative(filePath, rootDir).string();
}

PickMode::PickMode(Mode mode)
    : _mode(mode)
{
    memset(_searchFilterBuffer, 0, sizeof(char) * SEARCH_BUFFER_SIZE);

    _iconCamera = Camera3D{
        .position = Vector3 { 4.0f, 4.0f, 4.0f },
        .target = Vector3Zero(),
        .up = Vector3 { 0.0f, -1.0f, 0.0f },
        .fovy = 45.0f,
        .projection = CAMERA_PERSPECTIVE
    };
}

std::shared_ptr<Assets::TexHandle> PickMode::GetPickedTexture() const
{
    assert(_mode == Mode::TEXTURES);
    if (_selectedFrame.filePath.empty())
        return Assets::GetTexture(std::filesystem::path(GetApp()->GetDefaultTexturePath()));
    else
        return Assets::GetTexture(_selectedFrame.filePath);
}

std::shared_ptr<Assets::ModelHandle> PickMode::GetPickedShape() const
{
    assert(_mode == Mode::SHAPES);
    if (_selectedFrame.filePath.empty())
        return Assets::GetModel(std::filesystem::path(GetApp()->GetDefaultShapePath()));
    else
        return Assets::GetModel(_selectedFrame.filePath);
}

RLTexture2D PickMode::_GetTexture(const std::filesystem::path path)
{
    if (_loadedTextures.find(path) == _loadedTextures.end())
    {
        // Before loading the texture, resize it to fit into the frame
        RLImage image = LoadImage(path.string().c_str());
        ImageResize(&image, ICON_SIZE, ICON_SIZE);
        RLTexture2D texture = LoadTextureFromImage(image);
        UnloadImage(image);
        _loadedTextures[path] = texture;
    }
    return _loadedTextures[path];
}

RLModel PickMode::_GetModel(const std::filesystem::path path)
{
    if (_loadedModels.find(path) == _loadedModels.end())
    {
        RLModel model = RLLoadModel(path.string().c_str());
        _loadedModels[path] = model;
        return model;
    }
    return _loadedModels[path];
}

RenderTexture PickMode::_GetIcon(const std::filesystem::path path)
{
    if (_loadedIcons.find(path) == _loadedIcons.end())
    {
        RenderTexture icon = LoadRenderTexture(ICON_SIZE, ICON_SIZE);
        _loadedIcons[path] = icon;
        return icon;
    }
    return _loadedIcons[path];
}

void PickMode::_GetFrames()
{
    _frames.clear();

    for (const std::filesystem::path path : _foundFiles)
    {
        Frame frame(path, _rootDir);
        frame.texture = RLTexture{
            .id = 0,
            .width = ICON_SIZE,
            .height = ICON_SIZE,
            .mipmaps = 0,
            .format = 0,
        };

        //Filter out files that don't contain the search term
        std::string lowerCaseLabel = TextToLower(frame.label.c_str());
        if (strlen(_searchFilterBuffer) > 0 &&
            lowerCaseLabel.find(TextToLower(_searchFilterBuffer)) == std::string::npos)
        {
            continue;
        }

        _frames.push_back(frame);
    }
}

void PickMode::OnEnter()
{
    // Get the paths to all assets if this hasn't been done already.
    _rootDir = (_mode == Mode::SHAPES) ? std::filesystem::path(GetApp()->GetShapesDir()) : std::filesystem::path(GetApp()->GetTexturesDir());
    if (_foundFiles.empty())
    {
        if (!std::filesystem::is_directory(_rootDir))
        {
            std::cerr << "Asset directory in settings is not a directory!" << std::endl;
            return;
        }

        for (auto const& entry : std::filesystem::recursive_directory_iterator{ _rootDir })
        {
            if (entry.is_directory() || !entry.is_regular_file()) continue;

            //Filter out files with the wrong extensions
            auto extensionStr = TextToLower(entry.path().extension().string().c_str());
            if (
                (_mode != Mode::TEXTURES || !TextIsEqual(extensionStr, ".png")) &&
                (_mode != Mode::SHAPES || !TextIsEqual(extensionStr, ".obj"))
                ) {
                continue;
            }

            _foundFiles.insert(entry.path());
        }
    }

    _GetFrames();
}

void PickMode::OnExit()
{
    for (const auto& pair : _loadedModels)
    {
        UnloadModel(pair.second);
    }
    _loadedModels.clear();

    for (const auto& pair : _loadedTextures)
    {
        UnloadTexture(pair.second);
    }
    _loadedTextures.clear();

    for (const auto& pair : _loadedIcons)
    {
        UnloadRenderTexture(pair.second);
    }
    _loadedIcons.clear();
}

void PickMode::Update(float deltaTime)
{
    if (_mode == Mode::SHAPES)
    {
        for (Frame& frame : _frames)
        {
            //Update/redraw the shape preview icons so that they spin
            //BeginTextureMode(_GetIcon(frame.filePath));
            //ClearBackground(BLACK);
            //BeginMode3D(_iconCamera);

            //DrawModelWiresEx(_GetModel(frame.filePath), Vector3Zero(), Vector3{ 0.0f, 1.0f, 0.0f }, float(GetTime() * 180.0f), Vector3One(), GREEN);

            //EndMode3D();
            //EndTextureMode();
        }
    }
}

void PickMode::Draw()
{
    const float WINDOW_UPPER_MARGIN = 24.0f;
    bool open = true;
    ImGui::SetNextWindowSize(ImVec2((float)GetWindowWidth(), (float)GetWindowHeight() - WINDOW_UPPER_MARGIN));
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(ImVec2(center.x, center.y + WINDOW_UPPER_MARGIN), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::Begin("##Pick Mode View", &open, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove))
    {
        ImVec2 windowSize = ImGui::GetItemRectSize();

        if (ImGui::InputText("Search", _searchFilterBuffer, SEARCH_BUFFER_SIZE))
        {
            _GetFrames();
        }

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0, 8.0));
        ImGui::Separator();
        ImGui::PopStyleVar(1);

        const int NUM_COLS = Max((int)((windowSize.x) / (FRAME_SIZE * 1.5f)), 1);
        const int NUM_ROWS = Max((int)ceilf(_frames.size() / (float)NUM_COLS), 1);

        if (ImGui::BeginTable("##Frames", NUM_COLS, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_ScrollY))
        {
            for (int r = 0; r < NUM_ROWS; ++r)
            {
                ImGui::TableNextRow();
                for (int c = 0; c < NUM_COLS; ++c)
                {
                    ImGui::TableNextColumn();

                    int frameIndex = c + r * NUM_COLS;
                    if (frameIndex >= _frames.size()) break;

                    std::filesystem::path filePath = _frames[frameIndex].filePath;

                    ImColor color = ImColor(1.0f, 1.0f, 1.0f);
                    if (_selectedFrame.filePath == _frames[frameIndex].filePath)
                    {
                        // Set color when selected to yellow
                        color = ImColor(1.0f, 1.0f, 0.0f);
                    }

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(color));
                    if (rlImGuiImageButton(filePath.string().c_str(), &_frames[frameIndex].texture))
                    {
                        _selectedFrame = _frames[frameIndex];
                    }
                    ImGui::PopStyleColor(1);

                    if (ImGui::IsItemVisible() && !IsTextureReady(_frames[frameIndex].texture))
                    {
                        switch (_mode)
                        {
                        case Mode::TEXTURES: _frames[frameIndex].texture = _GetTexture(filePath); break;
                        case Mode::SHAPES: _frames[frameIndex].texture = _GetIcon(filePath).texture; break;
                        }
                    }

                    ImGui::TextColored(color, _frames[frameIndex].label.c_str());
                }
            }
            ImGui::EndTable();
        }

        ImGui::End();
    }
}