/*
 * funmod_base.c
 *
 *  Created on: 02.05.2017
 *	  Author: Daniel Schaefer <git@danielschaefer.me>
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


MODULE_AUTHOR("Daniel Schaefer <git@danielschaefer.me>");
MODULE_DESCRIPTION("Sample driver with no purpose");
MODULE_LICENSE("GPL");


static int number = 42;
module_param(number, int, 0644);
MODULE_PARM_DESC(number, "Some number");


static int __init fun_init(void)
{
	printk(KERN_ALERT "Today's number is %d", number);
	return 0;
}

module_init(fun_init);
