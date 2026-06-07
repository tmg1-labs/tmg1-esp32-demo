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
#endif
