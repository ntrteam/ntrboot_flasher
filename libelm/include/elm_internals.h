/*
    elm_internals.h
    Copyright (C) 2010 yellow wood goblin

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __ELM_INTERNALS_H__
#define __ELM_INTERNALS_H__

#include <elm_bool.h>

#if _MAX_SS == 512    /* Single sector size */
#define ELM_SS(fs)  512U
#elif _MAX_SS == 1024 || _MAX_SS == 2048 || _MAX_SS == 4096 /* Multiple sector size */
#define ELM_SS(fs)  ((fs)->ssize)
#else
#error Wrong sector size.
#endif

#endif
