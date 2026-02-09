#ifndef PROMETHEUS_HISTORY_HELPER_H
#define PROMETHEUS_HISTORY_HELPER_H
#include <functional>
#include <imgui.h>
#include <vector>

#include "globals.h"

template <typename Value_Type>
class history_helper {
public:
    static inline Value_Type empty{};
    Value_Type& current_item = empty;

    void push_item(Value_Type item) {
        _forward_history.clear();
        _history.push_back(std::move(item));

        current_item = _history.back();
    }

    void history_back() {
        _forward_history.push_back(current_item);
        _history.pop_back();

        current_item = _history.back();
    }

    void history_forward() {
        _history.push_back(_forward_history.back());
        _forward_history.pop_back();

        current_item = _history.back();
    }

    void display_history_buttons() {
        bool has_history = _history.size() > 1;
        bool has_forward_history = _forward_history.size() > 0;

        if (!has_history)
            ImGui::BeginDisabled();

        if (ImGui::Button(EMOJI_BACK)) {
            history_back();
        }

        if (!has_history)
            ImGui::EndDisabled();
        ImGui::SameLine();
        if (!has_forward_history)
            ImGui::BeginDisabled();

        if (ImGui::Button(EMOJI_FORWARD)) {
            history_forward();
        }

        if (!has_forward_history)
            ImGui::EndDisabled();
    }
private:
    std::vector<Value_Type> _history;
    //first = last history item, last = latest history item
    std::vector<Value_Type> _forward_history;
};

#endif //PROMETHEUS_HISTORY_HELPER_H