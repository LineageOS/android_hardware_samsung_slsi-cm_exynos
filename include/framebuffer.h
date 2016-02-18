/*
 * Copyright (C) 2016 Christopher N. Hesse <raymanfx@gmail.com>
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

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <ExynosHWCModule.h>

#ifdef DECON_FB
typedef decon_win_config fb_win_config;
typedef decon_win_config_data fb_win_config_data;
#else
typedef s3c_fb_win_config fb_win_config;
typedef s3c_fb_win_config_data fb_win_config_data;
#endif

#endif //FRAMEBUFFER_H
