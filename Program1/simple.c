/*
=============================================================
Name	    : simple.c
Date	    : February 7 ,2017
Author	    : Marbo Cheng
Description : Be able to download Kernel Module with 5 
		Birthdays in a linked list and be able to 
		removel Kernel module which will remove
		birthdays on linked list and free the memory
		space.
Important   : Make sure simple.c and Makefile is on same place
Compile	    : 1- Run make on terminal
	      2- Make will generate simple.ko
	      3- Install by running "sudo insmod simple.ko"
	      4- Uninstall by running "sudo rmmod simple"
=============================================================
*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/slab.h>

//struct for birthday
struct birthday {
int day;
int month;
int year;
struct list_head list;
	};
	
static LIST_HEAD(birthday_list);

/* This function is called when the module is loaded. */
int simple_init(void)
{
	struct birthday *person;
	struct birthday *ptr;
       	printk(KERN_INFO "Loading Module\n");
	
	//creates a birthday struct for each person
	person = kmalloc(sizeof(*person), GFP_KERNEL);
	person->day =2;
	person->month = 8;
	person->year=1995;
	
	INIT_LIST_HEAD(&person->list);
	
	//add person to the end of the tail of the list
	list_add_tail(&person->list, &birthday_list);

	//creating 4 more birthday
	person = kmalloc(sizeof(*person), GFP_KERNEL);
	person->day =1;
	person->month = 7;
	person->year=1996;
	INIT_LIST_HEAD(&person->list);
	list_add_tail(&person->list, &birthday_list);
	
	person = kmalloc(sizeof(*person), GFP_KERNEL);
	person->day =31;
	person->month = 6;
	person->year=1997;
	INIT_LIST_HEAD(&person->list);
	list_add_tail(&person->list, &birthday_list);

	person = kmalloc(sizeof(*person), GFP_KERNEL);
	person->day =30;
	person->month = 5;
	person->year=1998;
	INIT_LIST_HEAD(&person->list);
	list_add_tail(&person->list, &birthday_list);

	person = kmalloc(sizeof(*person), GFP_KERNEL);
	person->day =28;
	person->month = 3;
	person->year=1999;
	INIT_LIST_HEAD(&person->list);

	list_add_tail(&person->list, &birthday_list);
	
	//traversing the linked list and print birthdays

	list_for_each_entry(ptr, &birthday_list, list) {
	//on each iteration ptr points to the next birthday struct
	printk(KERN_INFO "Adding Day: %d Month: %d Year: %d\n",
		ptr->day, ptr->month, ptr->year); 
}
       return 0;
}

/* This function is called when the module is removed. */
void simple_exit(void) {
	struct birthday *ptr, *next;
	printk(KERN_INFO "Removing Module\n");
	

	list_for_each_entry_safe(ptr,next, &birthday_list, list) {
	//on each iteration ptr points to the next birthday struct
	printk( KERN_INFO "Removing Day: %d  Month: %d Year:  %d\n",
			ptr->day, ptr->month, ptr->year); 
	//removing birthday from list
	list_del(&ptr->list);
	
	//free memory space
	kfree(ptr);
	}
	
}


/* Macros for registering module entry and exit points. */
module_init( simple_init );
module_exit( simple_exit );

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Module");
MODULE_AUTHOR("Marbo Cheng");

