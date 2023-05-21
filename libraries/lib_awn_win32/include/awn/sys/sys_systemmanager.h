#pragma once

namespace awn::sys {

    struct SystemManager {
        u32         active_processor_count;
        u32         process_core_count;
        u64         process_core_mask;
        u64         system_core_mask;
        SYSTEM_INFO system_info;
        
        static constexpr size_t MaximumSupportedProcessorCount = 64;
    };

    void InitializeSystemManager();
    
    bool IsSystemManagerInitialized();

    SystemManager *GetSystemManager();

    u32 GetProcessCoreCount();
    u32 GetCurrentThreadCoreNumber();
    
    SYSTEM_INFO *GetSystemInfo();
}
