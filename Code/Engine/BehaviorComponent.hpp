#pragma once

#include "EngineCommon.hpp"
#include "Common/Types.hpp"
#include "Common/Filesystem.hpp"

namespace x {
    class BehaviorComponent {
    public:
        BehaviorComponent() = default;

        void Load(const u64 id) {
            mId = id;
        }

        u64 GetScriptId() const {
            return mId;
        }

    private:
        u64 mId {0};
    };
}  // namespace x