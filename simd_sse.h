/*
 * Copyright 2017 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DIMSUM_SIMD_SSE_H_
#define DIMSUM_SIMD_SSE_H_

#include <smmintrin.h>

#include "simd.h"

namespace dimsum {
namespace detail {

// SSE (16 bytes)
using XMM = detail::Abi<detail::StoragePolicy::kXmm, 16>;
using HalfXMM = detail::Abi<detail::StoragePolicy::kXmm, 8>;

SIMD_SPECIALIZATION(int8, detail::StoragePolicy::kXmm, 16, __m128i)
SIMD_SPECIALIZATION(int16, detail::StoragePolicy::kXmm, 16, __m128i)
SIMD_SPECIALIZATION(int32, detail::StoragePolicy::kXmm, 16, __m128i)
SIMD_SPECIALIZATION(int64, detail::StoragePolicy::kXmm, 16, __m128i)
SIMD_SPECIALIZATION(uint8, detail::StoragePolicy::kXmm, 16, __m128i)
SIMD_SPECIALIZATION(uint16, detail::StoragePolicy::kXmm, 16, __m128i)
SIMD_SPECIALIZATION(uint32, detail::StoragePolicy::kXmm, 16, __m128i)
SIMD_SPECIALIZATION(uint64, detail::StoragePolicy::kXmm, 16, __m128i)
SIMD_SPECIALIZATION(float, detail::StoragePolicy::kXmm, 16, __m128)
SIMD_SPECIALIZATION(double, detail::StoragePolicy::kXmm, 16, __m128d)

SIMD_NON_NATIVE_SPECIALIZATION_ALL_SMALL_BYTES(detail::StoragePolicy::kXmm);
SIMD_NON_NATIVE_SPECIALIZATION(detail::StoragePolicy::kXmm, 8);
SIMD_NON_NATIVE_SPECIALIZATION(detail::StoragePolicy::kXmm, 32);
SIMD_NON_NATIVE_SPECIALIZATION(detail::StoragePolicy::kXmm, 64);
SIMD_NON_NATIVE_SPECIALIZATION(detail::StoragePolicy::kXmm, 128);

template <typename T>
struct LoadImpl<T, detail::HalfXMM, flags::vector_aligned_tag> {
  static Simd<T, detail::HalfXMM> Apply(const T* buffer) {
    Simd<T, detail::HalfXMM> ret;
    memcpy(&ret.storage_, buffer, sizeof(ret));
    return ret;
  }
};

template <typename T, size_t kNumBytes>
struct LoadImpl<T, detail::Abi<detail::StoragePolicy::kXmm, kNumBytes>,
                flags::vector_aligned_tag> {
  static Simd<T, detail::Abi<detail::StoragePolicy::kXmm, kNumBytes>> Apply(
      const T* buffer) {
    Simd<T, detail::Abi<detail::StoragePolicy::kXmm, kNumBytes>> ret;
    __m128i ret1[sizeof(ret) / 16];
    for (int i = 0; i < sizeof(ret) / 16; i++)
      ret1[i] = _mm_load_si128(reinterpret_cast<const __m128i*>(buffer) + i);
    memcpy(&ret.storage_, &ret1, sizeof(ret1));
    return ret;
  }
};

}  // namespace detail

#ifndef __AVX2__
template <typename T>
using NativeSimd = Simd<T, detail::XMM>;
#endif

template <typename T>
using Simd128 = Simd<T, detail::XMM>;

template <typename T>
using Simd64 = Simd<T, detail::HalfXMM>;

template <>
inline Simd<int8, detail::XMM> abs(Simd<int8, detail::XMM> simd) {
  return _mm_abs_epi8(simd.raw());
}

template <>
inline Simd<int16, detail::XMM> abs(Simd<int16, detail::XMM> simd) {
  return _mm_abs_epi16(simd.raw());
}

template <>
inline Simd<int32, detail::XMM> abs(Simd<int32, detail::XMM> simd) {
  return _mm_abs_epi32(simd.raw());
}

#ifdef __AVX512VL__
template <>
inline Simd<int64, detail::XMM> abs(Simd<int64, detail::XMM> simd) {
  return _mm_abs_epi64(simd.raw());
}
#else
template <>
inline Simd<int64, detail::XMM> abs(Simd<int64, detail::XMM> simd) {
  return Simd<int64, detail::XMM>::list(std::abs(simd[0]), std::abs(simd[1]));
}
#endif

// ::abs for floating points is implemented by bit_and each lane with shr(-1, 1)
// to clear the sign bit (the most significant bit).
template <>
inline Simd<float, detail::XMM> abs(Simd<float, detail::XMM> simd) {
  return bit_cast<float>(bit_cast<uint32>(simd) &
                         Simd<uint32, detail::XMM>(~(1u << 31)));
}

template <>
inline Simd<double, detail::XMM> abs(Simd<double, detail::XMM> simd) {
  return bit_cast<double>(bit_cast<uint64>(simd) &
                          Simd<uint64, detail::XMM>(~(1ull << 63)));
}

template <>
inline Simd<float, detail::XMM> reciprocal_estimate(
    Simd<float, detail::XMM> simd) {
  return _mm_rcp_ps(simd.raw());
}

template <>
inline Simd<float, detail::XMM> sqrt(Simd<float, detail::XMM> simd) {
  return _mm_sqrt_ps(simd.raw());
}

template <>
inline Simd<double, detail::XMM> sqrt(Simd<double, detail::XMM> simd) {
  return _mm_sqrt_pd(simd.raw());
}

template <>
inline Simd<float, detail::XMM> reciprocal_sqrt_estimate(
    Simd<float, detail::XMM> simd) {
  return _mm_rsqrt_ps(simd.raw());
}

template <>
inline Simd<int8, detail::XMM> add_saturated(Simd<int8, detail::XMM> lhs,
                                             Simd<int8, detail::XMM> rhs) {
  return _mm_adds_epi8(lhs.raw(), rhs.raw());
}

template <>
inline Simd<uint8, detail::XMM> add_saturated(Simd<uint8, detail::XMM> lhs,
                                              Simd<uint8, detail::XMM> rhs) {
  return _mm_adds_epu8(lhs.raw(), rhs.raw());
}

template <>
inline Simd<int16, detail::XMM> add_saturated(Simd<int16, detail::XMM> lhs,
                                              Simd<int16, detail::XMM> rhs) {
  return _mm_adds_epi16(lhs.raw(), rhs.raw());
}

template <>
inline Simd<uint16, detail::XMM> add_saturated(Simd<uint16, detail::XMM> lhs,
                                               Simd<uint16, detail::XMM> rhs) {
  return _mm_adds_epu16(lhs.raw(), rhs.raw());
}

template <>
inline Simd<int8, detail::XMM> sub_saturated(Simd<int8, detail::XMM> lhs,
                                             Simd<int8, detail::XMM> rhs) {
  return _mm_subs_epi8(lhs.raw(), rhs.raw());
}

template <>
inline Simd<uint8, detail::XMM> sub_saturated(Simd<uint8, detail::XMM> lhs,
                                              Simd<uint8, detail::XMM> rhs) {
  return _mm_subs_epu8(lhs.raw(), rhs.raw());
}

template <>
inline Simd<int16, detail::XMM> sub_saturated(Simd<int16, detail::XMM> lhs,
                                              Simd<int16, detail::XMM> rhs) {
  return _mm_subs_epi16(lhs.raw(), rhs.raw());
}

template <>
inline Simd<uint16, detail::XMM> sub_saturated(Simd<uint16, detail::XMM> lhs,
                                               Simd<uint16, detail::XMM> rhs) {
  return _mm_subs_epu16(lhs.raw(), rhs.raw());
}

template <>
inline Simd<int8, detail::XMM> min(Simd<int8, detail::XMM> lhs,
                                   Simd<int8, detail::XMM> rhs) {
  return _mm_min_epi8(lhs.raw(), rhs.raw());
}

template <>
inline Simd<int16, detail::XMM> min(Simd<int16, detail::XMM> lhs,
                                    Simd<int16, detail::XMM> rhs) {
  return _mm_min_epi16(lhs.raw(), rhs.raw());
}

template <>
inline Simd<int32, detail::XMM> min(Simd<int32, detail::XMM> lhs,
                                    Simd<int32, detail::XMM> rhs) {
  return _mm_min_epi32(lhs.raw(), rhs.raw());
}

template <>
inline Simd<uint8, detail::XMM> min(Simd<uint8, detail::XMM> lhs,
                                    Simd<uint8, detail::XMM> rhs) {
  return _mm_min_epu8(lhs.raw(), rhs.raw());
}

template <>
inline Simd<uint16, detail::XMM> min(Simd<uint16, detail::XMM> lhs,
                                     Simd<uint16, detail::XMM> rhs) {
  return _mm_min_epu16(lhs.raw(), rhs.raw());
}

template <>
inline Simd<uint32, detail::XMM> min(Simd<uint32, detail::XMM> lhs,
                                     Simd<uint32, detail::XMM> rhs) {
  return _mm_min_epu32(lhs.raw(), rhs.raw());
}

template <>
inline Simd<float, detail::XMM> min(Simd<float, detail::XMM> lhs,
                                    Simd<float, detail::XMM> rhs) {
  return _mm_min_ps(lhs.raw(), rhs.raw());
}

template <>
inline Simd<double, detail::XMM> min(Simd<double, detail::XMM> lhs,
                                     Simd<double, detail::XMM> rhs) {
  return _mm_min_pd(lhs.raw(), rhs.raw());
}

template <>
inline Simd<int8, detail::XMM> max(Simd<int8, detail::XMM> lhs,
                                   Simd<int8, detail::XMM> rhs) {
  return _mm_max_epi8(lhs.raw(), rhs.raw());
}

template <>
inline Simd<int16, detail::XMM> max(Simd<int16, detail::XMM> lhs,
                                    Simd<int16, detail::XMM> rhs) {
  return _mm_max_epi16(lhs.raw(), rhs.raw());
}

template <>
inline Simd<int32, detail::XMM> max(Simd<int32, detail::XMM> lhs,
                                    Simd<int32, detail::XMM> rhs) {
  return _mm_max_epi32(lhs.raw(), rhs.raw());
}

template <>
inline Simd<uint8, detail::XMM> max(Simd<uint8, detail::XMM> lhs,
                                    Simd<uint8, detail::XMM> rhs) {
  return _mm_max_epu8(lhs.raw(), rhs.raw());
}

template <>
inline Simd<uint16, detail::XMM> max(Simd<uint16, detail::XMM> lhs,
                                     Simd<uint16, detail::XMM> rhs) {
  return _mm_max_epu16(lhs.raw(), rhs.raw());
}

template <>
inline Simd<uint32, detail::XMM> max(Simd<uint32, detail::XMM> lhs,
                                     Simd<uint32, detail::XMM> rhs) {
  return _mm_max_epu32(lhs.raw(), rhs.raw());
}

template <>
inline Simd<float, detail::XMM> max(Simd<float, detail::XMM> lhs,
                                    Simd<float, detail::XMM> rhs) {
  return _mm_max_ps(lhs.raw(), rhs.raw());
}

template <>
inline Simd<double, detail::XMM> max(Simd<double, detail::XMM> lhs,
                                     Simd<double, detail::XMM> rhs) {
  return _mm_max_pd(lhs.raw(), rhs.raw());
}

template <>
inline Simd<int8, detail::XMM> pack_saturated(Simd<int16, detail::XMM> lhs,
                                              Simd<int16, detail::XMM> rhs) {
  return _mm_packs_epi16(lhs.raw(), rhs.raw());
}

template <>
inline Simd<int16, detail::XMM> pack_saturated(Simd<int32, detail::XMM> lhs,
                                               Simd<int32, detail::XMM> rhs) {
  return _mm_packs_epi32(lhs.raw(), rhs.raw());
}

template <>
inline Simd<uint8, detail::XMM> packu_saturated(Simd<int16, detail::XMM> lhs,
                                                Simd<int16, detail::XMM> rhs) {
  return _mm_packus_epi16(lhs.raw(), rhs.raw());
}

template <>
inline Simd<uint16, detail::XMM> packu_saturated(Simd<int32, detail::XMM> lhs,
                                                 Simd<int32, detail::XMM> rhs) {
  return _mm_packus_epi32(lhs.raw(), rhs.raw());
}

template <>
inline Simd<uint64, detail::XMM> reduce_add<uint64, 2>(
    Simd<uint8, detail::XMM> simd) {
  return _mm_sad_epu8(simd.raw(), Simd<uint8, detail::XMM>(0).raw());
}

template <>
inline Simd<int32, detail::XMM> reduce_add<int32, 4>(
    Simd<int16, detail::XMM> simd) {
  return _mm_madd_epi16(simd.raw(), Simd<int16, detail::XMM>(1).raw());
}

template <>
inline ResizeTo<Simd<uint64, detail::XMM>, 1> reduce_add<uint64, 1>(
    Simd<uint8, detail::XMM> simd) {
  return reduce_add<uint64, 1>(reduce_add<uint64, 2>(simd));
}

template <>
inline ResizeTo<Simd<uint64, detail::XMM>, 1> reduce_add<uint64, 1>(
    Simd<uint16, detail::XMM> simd) {
  uint64 even_sum = reduce_add<uint64, 1>(
      bit_cast<uint8>(simd & Simd<uint16, detail::XMM>(0x00ff)))[0];
  uint64 odd_sum = reduce_add<uint64, 1>(
      bit_cast<uint8>(simd & Simd<uint16, detail::XMM>(0xff00)))[0];
  return (odd_sum << 8) + even_sum;
}

template <>
inline float reduce<float, detail::XMM, std::plus<float>>(
    const Simd<float, detail::XMM>& simd, std::plus<float>) {
  return (simd[0] + simd[2]) + (simd[1] + simd[3]);
}

template <>
inline Simd<int32, detail::XMM> mul_sum(Simd<int16, detail::XMM> lhs,
                                        Simd<int16, detail::XMM> rhs,
                                        Simd<int32, detail::XMM> acc) {
  return _mm_add_epi32(acc.raw(), _mm_madd_epi16(lhs.raw(), rhs.raw()));
}

// _MM_FROUND_TO_NEAREST_INT specifies round-to-even.
template <>
inline Simd<float, detail::XMM> round(Simd<float, detail::XMM> simd) {
  return _mm_round_ps(simd.raw(),
                      _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
}

template <>
inline Simd<double, detail::XMM> round(Simd<double, detail::XMM> simd) {
  return _mm_round_pd(simd.raw(),
                      _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
}

template <>
inline Simd<int32, detail::XMM> round_to_integer(
    Simd<float, detail::XMM> simd) {
  return _mm_cvtps_epi32(simd.raw());
}

template <typename T>
Simd<ScaleBy<T, 2>, detail::XMM> mul_widened(Simd<T, detail::HalfXMM> lhs,
                                             Simd<T, detail::HalfXMM> rhs) {
  return simd_cast<ScaleBy<T, 2>>(lhs) * simd_cast<ScaleBy<T, 2>>(rhs);
}

}  // namespace dimsum

#ifndef __AVX2__
#undef SIMD_SPECIALIZATION
#endif  // __AVX2__

#endif  // DIMSUM_SIMD_SSE_H_
