#pragma once
#include <stddef.h>
#include <stdint.h>

// コールバックベースI/O: Arduino Stream・ファイル・メモリバッファを抽象化する
// 戻り値: 成功時は転送バイト数 (read/write)、失敗時は負値

#ifdef __cplusplus
extern "C" {
#endif

typedef int  (*Tmg1ReadFn) (void* ctx, uint8_t* buf, size_t len);
typedef int  (*Tmg1WriteFn)(void* ctx, const uint8_t* buf, size_t len);
typedef long (*Tmg1TellFn) (void* ctx);
typedef int  (*Tmg1SeekFn) (void* ctx, long offset, int origin);

typedef struct {
    void*        ctx;
    Tmg1ReadFn   read;
    Tmg1WriteFn  write; // NULL: 読み取り専用
    Tmg1TellFn   tell;  // NULL可
    Tmg1SeekFn   seek;  // NULL可
} Tmg1Stream;

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
// C++ヘルパー: メモリバッファからTmg1Streamを生成する
// Arduinoでは使用不可。デスクトップ(native)ビルド専用。
struct Tmg1MemReadCtx {
    const uint8_t* data;
    size_t         size;
    size_t         pos;
};

inline int tmg1_mem_read(void* ctx, uint8_t* buf, size_t len) {
    auto* c = static_cast<Tmg1MemReadCtx*>(ctx);
    size_t avail = c->size - c->pos;
    if (len > avail) len = avail;
    if (len == 0) return 0;
    __builtin_memcpy(buf, c->data + c->pos, len);
    c->pos += len;
    return (int)len;
}

struct Tmg1MemWriteCtx {
    uint8_t* data;
    size_t   capacity;
    size_t   pos;
};

inline int tmg1_mem_write(void* ctx, const uint8_t* buf, size_t len) {
    auto* c = static_cast<Tmg1MemWriteCtx*>(ctx);
    size_t avail = c->capacity - c->pos;
    if (len > avail) len = avail;
    if (len == 0) return 0;
    __builtin_memcpy(c->data + c->pos, buf, len);
    c->pos += len;
    return (int)len;
}

// stdio (FILE*) ベースのTmg1Stream。デスクトップテスト用。
// Arduinoビルドでは <cstdio> が使えないため除外する。
#if !defined(ARDUINO) && !defined(IRAM_ATTR)
#include <cstdio>

struct Tmg1FileCtx {
    FILE* fp;
};

inline int tmg1_file_read(void* ctx, uint8_t* buf, size_t len) {
    auto* c = static_cast<Tmg1FileCtx*>(ctx);
    return (int)fread(buf, 1, len, c->fp);
}

inline int tmg1_file_write(void* ctx, const uint8_t* buf, size_t len) {
    auto* c = static_cast<Tmg1FileCtx*>(ctx);
    return (int)fwrite(buf, 1, len, c->fp);
}

inline long tmg1_file_tell(void* ctx) {
    auto* c = static_cast<Tmg1FileCtx*>(ctx);
    return ftell(c->fp);
}

inline int tmg1_file_seek(void* ctx, long offset, int origin) {
    auto* c = static_cast<Tmg1FileCtx*>(ctx);
    return fseek(c->fp, offset, origin);
}

// 読み取り専用Tmg1Streamを生成する (ファイルのopen/closeは呼び出し元の責任)
inline Tmg1Stream tmg1_stream_from_file_read(Tmg1FileCtx& ctx, FILE* fp) {
    ctx.fp = fp;
    Tmg1Stream s = {};
    s.ctx  = &ctx;
    s.read = tmg1_file_read;
    s.tell = tmg1_file_tell;
    s.seek = tmg1_file_seek;
    return s;
}

// 書き込み専用Tmg1Streamを生成する
inline Tmg1Stream tmg1_stream_from_file_write(Tmg1FileCtx& ctx, FILE* fp) {
    ctx.fp = fp;
    Tmg1Stream s = {};
    s.ctx   = &ctx;
    s.write = tmg1_file_write;
    return s;
}
#endif // !ARDUINO

#endif // __cplusplus
