/*
 * funmod_extension.c
 *
 *  Created on: 02.05.2017 *	  Author: Daniel Schaefer <git@danielschaefer.me>
 *
 *  Module for playing around with linux kernel modules.
 *  Copyright (C) 2018 Daniel Schaefer <git@danielschaefer.me>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>


static void __exit fun_exit(void)
{
	printk(KERN_ALERT "Unloading fun driver");
}
module_exit(fun_exit);
