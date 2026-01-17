// Hazno - 2026

#pragma once

#include <imgui.h>
#include <span>
#include <variant>
#include <vector>

#include "idadefs.h"

#define ENUMFLAGS_DRAW(val, count, ...)                                              \
    do {                                                                             \
        static constexpr std::pair<uint8, const char*> enf_names[] = { __VA_ARGS__ };   \
        Editor::Drawers::EnumFlags::Draw<count>(val, std::span{enf_names, sizeof(enf_names) / sizeof(enf_names[0])}); \
    } while (0)

namespace Editor::Drawers
{
    class EnumFlags
    {
        static constexpr ImVec2 s_checkboxSpacing{ 3.0f, 3.0f };
        static constexpr ImVec2 s_alignment{ 0.5f, 0.5f };

        template <uint8 Count>
        static const std::array<const char*, Count>& DefaultHexLabels()
        {
            static std::array<std::array<char, 4>, Count> storage{};
            static std::array<const char*, Count> ptrs{};
            static bool inited = false;

            if (!inited) {
                for (uint8 i = 0; i < Count; i++) {
                    std::snprintf(storage[i].data(), storage[i].size(), "%d", i);
                    ptrs[i] = storage[i].data();
                }
                inited = true;
            }

            return ptrs;
        }

        static void PushColours(const bool enabled)
        {
            static constexpr ImVec4 s_colourOff{ 0.7f, 0.3f, 0.3f, 1.0f };
            static constexpr ImVec4 s_colourOn{ 0.3f, 0.7f, 0.3f, 1.0f };
            static constexpr ImVec4 s_colourBase{ 1.0f, 1.0f, 1.0f, 0.4f };
            static constexpr ImVec4 s_colourHovered{ 1.0f, 1.0f, 1.0f, 0.6f };
            static constexpr ImVec4 s_colourActive{ 1.0f, 1.0f, 1.0f, 0.7f };

            const auto col = enabled ? s_colourOn : s_colourOff;
            ImGui::PushStyleColor(ImGuiCol_Button, col * s_colourBase);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, col * s_colourHovered);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, col * s_colourActive);
        }

        public:
            template <uint8 Count, class T> requires std::is_integral_v<T>
            static void Draw(T* value, std::span<const std::pair<uint8, const char*>> names)
            {
                static_assert(Count < 64, "EnumFlagsDrawer only supports up to 64 flags.");

                if (!value) {
                    ImGui::TextDisabled("NULL!");
                    return;
                }

                auto labels = DefaultHexLabels<Count>();
                for (auto [bit, label] : names) {
                    if (bit < Count && label && *label)
                        labels[bit] = label;
                }

                ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, s_checkboxSpacing);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, s_alignment);

                if (ImGui::BeginTable("Flags", 1 + static_cast<int>(ImGui::GetContentRegionAvail().x / 180.0f), ImGuiTableFlags_NoSavedSettings)) {
                    for (uint8 i = 0; i < Count; i++) {
                        ImGui::PushID(i);
                        ImGui::TableNextColumn();

                        PushColours((*value & 1UL << i) != 0);
                        if (ImGui::Button(labels[i], {ImGui::GetColumnWidth(), 22.0f})) {
                            *value ^= 1UL << i;
                        }

                        ImGui::PopStyleColor(3);
                        ImGui::PopID();
                    }

                    ImGui::EndTable();
                }

                ImGui::PopStyleVar(3);
            }

            template <uint8 Count, class T> requires std::is_integral_v<T>
            static void Draw(T* value, const std::initializer_list<std::pair<uint8, const char*>> names)
            {
                Draw<Count>(value, std::span{names.begin(), names.size()});
            }
    };
}