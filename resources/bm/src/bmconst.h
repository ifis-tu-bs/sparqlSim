#ifndef BMCONST__H__INCLUDED__
#define BMCONST__H__INCLUDED__
/*
Copyright(c) 2002-2017 Anatoliy Kuznetsov(anatoliy_kuznetsov at yahoo.com)

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

You have to explicitly mention BitMagic project in any derivative product,
its WEB Site, published materials, articles or any other work derived from this
project or based on our code or know-how.

For more information please visit:  http://bitmagic.io

*/

namespace bm
{

#if defined(_WIN32) || defined (_WIN64)

typedef unsigned __int64 id64_t;

#else

typedef unsigned long long int id64_t;

#endif

typedef unsigned int   id_t;
typedef unsigned int   word_t;
typedef unsigned short short_t;



const unsigned id_max = 0xFFFFFFFF;

// Data Block parameters

const unsigned set_block_size  = 2048u;
const unsigned set_block_shift = 16u;
const unsigned set_block_mask  = 0xFFFFu;
const unsigned set_blkblk_mask = 0xFFFFFFu;

const unsigned set_block_plain_size = set_block_size / 32u;
const unsigned set_block_plain_cnt = (unsigned)(sizeof(bm::word_t) * 8u);

// Word parameters

const unsigned set_word_shift = 5u;
const unsigned set_word_mask  = 0x1Fu;


// GAP related parameters.

typedef unsigned short gap_word_t;

const unsigned gap_max_buff_len = 1280;
const unsigned gap_max_bits = 65536;
const unsigned gap_equiv_len = (unsigned)
   ((sizeof(bm::word_t) * bm::set_block_size) / sizeof(gap_word_t));
const unsigned gap_levels = 4;
const unsigned gap_max_level = bm::gap_levels - 1;


// Block Array parameters

const unsigned set_array_size = 256u;
const unsigned set_array_shift = 8u;
const unsigned set_array_mask  = 0xFFu;
const unsigned set_total_blocks = (bm::set_array_size * bm::set_array_size);

const unsigned bits_in_block = bm::set_block_size * (unsigned)(sizeof(bm::word_t) * 8);
const unsigned bits_in_array = bm::bits_in_block * bm::set_array_size;


#if defined(BM64OPT) || defined(BM64_SSE4)

typedef id64_t  wordop_t;
const id64_t    all_bits_mask = 0xffffffffffffffff;
const unsigned set_block_size_op  = bm::set_block_size / 2;

#else

typedef word_t wordop_t;
const word_t all_bits_mask = 0xffffffff;
const unsigned set_block_size_op  = bm::set_block_size;

#endif

# define BM_DECLARE_TEMP_BLOCK(x)  unsigned BM_VECT_ALIGN x[bm::set_block_size] BM_VECT_ALIGN_ATTR;


/*!
   @brief Block allocation strategies.
   @ingroup bvector
*/
enum strategy
{
    BM_BIT = 0, //!< No GAP compression strategy. All new blocks are bit blocks.
    BM_GAP = 1  //!< GAP compression is ON.
};


/*!
    @brief set representation variants
    @internal
*/
enum set_representation
{
    set_bitset  = 0,  //!< Simple bitset
    set_gap     = 1,  //!< GAP-RLE compression
    set_array1  = 2,  //!< array of set 1 values
    set_array0  = 3   //!< array of 0 values
};

/**
    Internal structure. Copyright information.
*/
template<bool T> struct _copyright
{
    static const char _p[];
    static const unsigned _v[3];
};

template<bool T> const char _copyright<T>::_p[] = 
    "BitMagic C++ Library. v.3.10.0 (c) 2002-2017 Anatoliy Kuznetsov.";
template<bool T> const unsigned _copyright<T>::_v[3] = {3, 9, 0};


template<bool T> struct DeBruijn_bit_position
{
    static const unsigned _multiply[32];
};

template<bool T>
const unsigned DeBruijn_bit_position<T>::_multiply[32] = { 
  0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8, 
  31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
};

/** Structure keeps index of first right 1 bit for every byte.  
    @ingroup bitfunc 
*/
template<bool T> struct first_bit_table
{
    static const signed char _idx[256];
};

template<bool T>
const signed char first_bit_table<T>::_idx[256] = {
  -1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
};

//---------------------------------------------------------------------

/** Structure to aid in counting bits
    table contains count of bits in 0-255 diapason of numbers

   @ingroup bitfunc
*/
template<bool T> struct bit_count_table 
{
  static const unsigned char _count[256];
};

template<bool T>
const unsigned char bit_count_table<T>::_count[256] = {
    0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
    1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
    1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
    2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
    1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
    2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
    2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
    3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8
};

//---------------------------------------------------------------------

/** Structure keeps all-left/right ON bits masks. 
    @ingroup bitfunc 
*/
template<bool T> struct block_set_table
{
    static const unsigned _left[32];
    static const unsigned _right[32];
};

template<bool T>
const unsigned block_set_table<T>::_left[32] = {
    0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff, 0x1ff, 0x3ff, 0x7ff,
    0xfff, 0x1fff, 0x3fff, 0x7fff, 0xffff, 0x1ffff, 0x3ffff, 0x7ffff,
    0xfffff, 0x1fffff, 0x3fffff, 0x7fffff, 0xffffff, 0x1ffffff, 0x3ffffff,
    0x7ffffff, 0xfffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff
};

template<bool T>
const unsigned block_set_table<T>::_right[32] = {
    0xffffffff, 0xfffffffe, 0xfffffffc, 0xfffffff8, 0xfffffff0,
    0xffffffe0, 0xffffffc0, 0xffffff80, 0xffffff00, 0xfffffe00,
    0xfffffc00, 0xfffff800, 0xfffff000, 0xffffe000, 0xffffc000,
    0xffff8000, 0xffff0000, 0xfffe0000, 0xfffc0000, 0xfff80000,
    0xfff00000, 0xffe00000, 0xffc00000, 0xff800000, 0xff000000,
    0xfe000000, 0xfc000000, 0xf8000000, 0xf0000000, 0xe0000000,
    0xc0000000, 0x80000000
};

//---------------------------------------------------------------------



/*! @brief Default GAP lengths table.
    @ingroup gapfunc
*/
template<bool T> struct gap_len_table
{
    static const gap_word_t _len[bm::gap_levels];
};

template<bool T>
const gap_word_t gap_len_table<T>::_len[bm::gap_levels] = 
                { 128, 256, 512, bm::gap_max_buff_len }; 


/*! @brief Alternative GAP lengths table. 
    Good for for memory saver mode and very sparse bitsets.

    @ingroup gapfunc
*/
template<bool T> struct gap_len_table_min
{
    static const gap_word_t _len[bm::gap_levels];
};

template<bool T>
const gap_word_t gap_len_table_min<T>::_len[bm::gap_levels] = 
                                { 32, 96, 128, 512 }; 


/*! @brief Non-linear size growth GAP lengths table.
    @ingroup gapfunc
*/
template<bool T> struct gap_len_table_nl
{
    static const gap_word_t _len[bm::gap_levels];
};

template<bool T>
const gap_word_t gap_len_table_nl<T>::_len[bm::gap_levels] =
                { 32, 128, 512, bm::gap_max_buff_len };



} // namespace

#endif

