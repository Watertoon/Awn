#pragma once

namespace awn::font {

    class CharacterStreamReader {
        private:
            const char *m_string;
            m_read_callback;
        public:
            constexpr ;
    };

    struct CharacterWidths {
        u8 left_width;
        u8 glyph_width;
        u8 char_width;
    };

    class Font {
        public:
            constexpr Font() {/*...*/}

            virtual void Finalize() {/*...*/}
            
            virtual void GetCharacterEncoding();

            virtual void GetCharacterWidths();
    };

    class BffntFont : public Font {
        private:
            vp::res::ResBffnt             *m_bffnt;
            TextureArchiveMemoryRelocator  m_font_texture_relocator;
            vp::res::ResBntxTexture       *m_font_texture;
        public:
            constexpr BffntFont() {/*...*/}

            bool Initialize(void *bffnt_file) {

                /* Cast bffnt file */
                vp::res::ResBffnt *bffnt = vp::res::ResBffnt::ResCast(bffnt_file);
                if (bffnt->IsValid() == false) { return false; }

                /* Relocate texture */
                vp::res::ResBntx *bntx = ;
                m_font_texture_relocator.Initialize(bntx);

                /* Get font texture */
                m_font_texture = ;

                return true;
            }
    };
}
