#include "tmg1_c.h"
#include "tmg1/decoder.h"
#include "tmg1/encoder.h"
#include <new>

// opaque 型の実体
struct Tmg1DecoderOpaque {
    Tmg1Stream       stream;
    tmg1::Decoder    decoder;
};

struct Tmg1EncoderOpaque {
    Tmg1Stream       stream;
    tmg1::Encoder    encoder;
    explicit Tmg1EncoderOpaque(const tmg1::EncodeConfig& cfg)
        : encoder(cfg) {}
};

// --- デコーダ ---

Tmg1Decoder* tmg1_decoder_create(Tmg1Stream* stream) {
    auto* obj = new (std::nothrow) Tmg1DecoderOpaque();
    if (!obj) return nullptr;
    obj->stream = *stream;
    if (obj->decoder.begin(obj->stream) != tmg1::Error::None) {
        delete obj;
        return nullptr;
    }
    return obj;
}

void tmg1_decoder_destroy(Tmg1Decoder* dec) {
    delete dec;
}

int tmg1_decoder_decode_frame(Tmg1Decoder* dec, uint8_t* out, size_t out_size) {
    return (int)dec->decoder.decodeFrame(out, out_size);
}

uint16_t tmg1_decoder_width(const Tmg1Decoder* dec) {
    return dec->decoder.getWidth();
}

uint16_t tmg1_decoder_height(const Tmg1Decoder* dec) {
    return dec->decoder.getHeight();
}

uint16_t tmg1_decoder_timebase_num(const Tmg1Decoder* dec) {
    return dec->decoder.getTimebaseNum();
}

uint16_t tmg1_decoder_timebase_den(const Tmg1Decoder* dec) {
    return dec->decoder.getTimebaseDen();
}

uint32_t tmg1_decoder_last_pts_delta(const Tmg1Decoder* dec) {
    return dec->decoder.getLastPtsDelta();
}

// --- エンコーダ ---

Tmg1Encoder* tmg1_encoder_create(Tmg1Stream* stream, const Tmg1EncodeConfig* config) {
    tmg1::EncodeConfig cfg = {};
    cfg.width         = config->width;
    cfg.height        = config->height;
    cfg.timebaseNum   = config->timebaseNum;
    cfg.timebaseDen   = config->timebaseDen;
    cfg.keyInterval   = config->keyInterval;
    cfg.msbFirst      = config->msbFirst      != 0;
    cfg.useRangeCoder = config->useRangeCoder != 0;
    cfg.deltaEnabled  = config->deltaEnabled  != 0;

    auto* obj = new (std::nothrow) Tmg1EncoderOpaque(cfg);
    if (!obj) return nullptr;
    obj->stream = *stream;
    if (obj->encoder.begin(obj->stream) != tmg1::Error::None) {
        delete obj;
        return nullptr;
    }
    return obj;
}

void tmg1_encoder_destroy(Tmg1Encoder* enc) {
    delete enc;
}

int tmg1_encoder_encode_frame(Tmg1Encoder* enc, const uint8_t* frame, size_t frame_size) {
    return (int)enc->encoder.encodeFrame(frame, frame_size);
}

int tmg1_encoder_finish(Tmg1Encoder* enc) {
    return (int)enc->encoder.finish();
}
