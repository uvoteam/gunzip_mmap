#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <zlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define OUT_BUFFER_MULT 5

off_t get_file_size(int fd);
void decompress(Bytef *src, int src_len, void *dst, int dst_len);
int decompress_block(z_stream *strm, void *dst);

int main(int argc, char *argv[])
{
    assert(argc == 2);
    int fd = open(argv[1], O_RDONLY, 0);
    assert(fd != -1);
    off_t file_size = get_file_size(fd);
    off_t out_size = file_size * OUT_BUFFER_MULT;
    unsigned char *out = malloc(out_size * sizeof(char));
    assert(out != NULL);
    void *data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);
    assert(data != NULL);
    decompress(data, file_size, out, out_size);
    free(out);
    assert(munmap(data, file_size) == 0);
    close(fd);

    return 0;
}

off_t get_file_size(int fd)
{
    struct stat st;
    assert(fstat(fd, &st) != -1);

    return st.st_size;
}

void decompress(Bytef *src, int src_len, void *dst, int dst_len)
{
    z_stream strm  = {0};
    strm.zalloc = Z_NULL;
    strm.zfree  = Z_NULL;
    strm.opaque = Z_NULL;

    while (src_len > 0)
    {
        strm.total_in  = strm.avail_in  = src_len;
        strm.total_out = strm.avail_out = dst_len;
        strm.next_in   = src;
        strm.next_out  = (Bytef *) dst;
        assert(inflateInit2(&strm, (MAX_WBITS + 16)) == Z_OK);
        decompress_block(&strm, dst);
        unsigned int processed = src_len - strm.avail_in;
        src_len -= processed;
        src += processed;
    }
}

int decompress_block(z_stream *strm, void *dst)
{
    int ret;
    do
    {
        ret = inflate(strm, Z_NO_FLUSH);
        assert(ret != Z_STREAM_ERROR && ret != Z_NEED_DICT && ret != Z_MEM_ERROR && ret != Z_DATA_ERROR);
        assert(strm->avail_out > 0);
    }
    while (ret != Z_STREAM_END);
    inflateEnd(strm);
    assert(write(1, dst, strm->total_out) == strm->total_out);
    return 0;
}
