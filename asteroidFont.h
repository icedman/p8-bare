/** \file
 * Super simple font from Asteroids.
 *
 * http://www.edge-online.com/wp-content/uploads/edgeonline/oldfiles/images/feature_article/2009/05/asteroids2.jpg
 *
 * Downloaded 2018-5-25 from https://github.com/osresearch/vst/blob/master/teensyv/asteroids_font.c
 */

#pragma once

#include <Arduino.h>

void initAsteroids();
int asteroidDrawChar(float x, float y, char c, float size, int clr = 0xff << 11 | 0xff << 5 | 0xff, int bg = 0, bool extentsOnly = false);
int asteroidDrawString(float x, float y, char *str, float size, int clr = 0xff << 11 | 0xff << 5 | 0xff, int bg = 0, bool extentsOnly = false);