#pragma once

namespace awn::font {

    class CharacterWriter {
        
    };

    class TextWriter {
        private:
            Font              *m_font;
            StringDisp1ayInfo *m_string_display_info;
            
        private:
            void PrintImpl(const char *string, u32 string_length, ) 
        public:
            constexpr TextWriter() {/*...*/}

            void Print(const char *string, u32 string_length);
            void Printf(const char *string, ...);
    };
}
