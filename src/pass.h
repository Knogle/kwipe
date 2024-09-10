/*
 *  pass.h: Routines that read and write patterns to block devices.
 *
 *  Copyright Darik Horn <dajhorn-dban@vanadac.com>.
 *
 *  Modifications to original dwipe Copyright Andy Beverley <andy@andybev.com>
 *
 *  This program is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free Software
 *  Foundation, version 2.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along with
 *  this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef PASS_H_
#define PASS_H_

int kwipe_random_pass( kwipe_context_t* c );
int kwipe_random_verify( kwipe_context_t* c );
int kwipe_static_pass( kwipe_context_t* c, kwipe_pattern_t* pattern );
int kwipe_static_verify( kwipe_context_t* c, kwipe_pattern_t* pattern );

void test_functionn( int count, kwipe_context_t** c );

#endif /* PASS_H_ */
