//
// internal.h
//
// Copyright (c) 2017 Hurzhii Artem, Demicev Alexandr, Denisov Artem, Chufarov Evgeny
//

#ifndef IFJ17_INTERNAL_H
#define IFJ17_INTERNAL_H

// branch prediction

#if defined(__GNUC__) && (__GNUC__ > 2) && defined(__OPTIMIZE__)
#define likely(expr) __builtin_expect((expr), 1)
#define unlikely(expr) __builtin_expect((expr), 0)
#else
#define likely(expr) (expr)
#define unlikely(expr) (expr)
#endif

#endif /* IFJ17_INTERNAL_H */
