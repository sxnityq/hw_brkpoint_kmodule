#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>


static struct perf_event *__percpu *event;
static unsigned long long addr;

static struct kobject           *addr_obj;
static struct perf_event_attr   event_attr;


/* ##################################
 *  HW BREAKPOINT   RELATED     SHIT
 * #################################
 */
static void triggered(struct perf_event *event, struct perf_sample_data *data, struct pt_regs *regs)
{
    pr_info("[%s]: triggered at address %llX\tHOT!!!\n", module_name(THIS_MODULE), addr);
    return;
}


/* Function to configure the perf_event_attr structure */
static void fill_attr(struct perf_event_attr *attr)
{
    attr->type      = PERF_TYPE_BREAKPOINT;
    attr->bp_addr   = addr;
    attr->bp_type   = HW_BREAKPOINT_RW;
    attr->bp_len    = HW_BREAKPOINT_LEN_8;
}


/*  ##########################
 *   SYSFS   RELATED     SHIT
 *  ##########################
*/
static ssize_t addr_show(struct kobject *kobj,
    struct kobj_attribute *attr, char *buf)
{
    pr_info("[%s]: Read an address %llX from sysfs \n", module_name(THIS_MODULE), addr);
    return sprintf(buf, "%llX\n", addr);
}


static ssize_t addr_store(struct kobject *kobj,
    struct kobj_attribute *attr, char *buf, size_t count)
{
    unsigned long long tmp_addr; //  For testing if addr is correct
    int status;

    status = kstrtoull(buf, 16, &tmp_addr);

    if (status == -EINVAL){
        pr_err("[%s]: write failed. Given and invalid 16base address\n", module_name(THIS_MODULE));
        goto err;
    } 
    
    if (status == -ERANGE){
        pr_err("[%s]: write failed. f*cking overflow detected\n", module_name(THIS_MODULE));
        goto err;
    }

    if (!IS_ERR(event)){
        unregister_wide_hw_breakpoint(event);
        addr = tmp_addr;
        event_attr.bp_addr = addr;
        event = register_wide_hw_breakpoint(&event_attr, triggered, NULL);
        pr_info("[%s]: write an address %llX to sysfs \n", module_name(THIS_MODULE), addr);
        if (IS_ERR(event)) {
            pr_err("[%s]: Failed to recreate hw breakpoint;\tstatus: %ld\n", module_name(THIS_MODULE), PTR_ERR(event));
            /*
                TODO: kernel panic or unload the f*cking module
            */
        }
    }
    
    err:
        return count;
}

static struct kobj_attribute addr_obj_attribute =
    __ATTR(addr, 0660, addr_show, (void *)addr_store);


static int __init init_mmodule(void)
{
    int error = 0;
    addr = 0xffffffff8257eac0;

    pr_info("[%s]: Initializing module\n", module_name(THIS_MODULE));

    addr_obj = kobject_create_and_add(module_name(THIS_MODULE), kernel_kobj);
    if (!addr_obj)
        return -ENOMEM;

    error = sysfs_create_file(addr_obj, &addr_obj_attribute.attr);
    if (error)
        pr_info("failed to create the %s file in /sys/kernel/%s\n", module_name(THIS_MODULE), module_name(THIS_MODULE));


    hw_breakpoint_init(&event_attr);
    fill_attr(&event_attr);

    event = register_wide_hw_breakpoint(&event_attr, triggered, NULL);
    if (IS_ERR(event)) {
        pr_err("[%s]: Failed to create perf event;\tstatus: %ld\n", module_name(THIS_MODULE), PTR_ERR(event));
        return PTR_ERR(event);
    }

    pr_info("[%s]: Set breakpoint at address %llX\n", module_name(THIS_MODULE), addr);
    return 0;
}


static void __exit exit_mmodule(void)
{
    pr_info("[%s]: Exiting module...\n", module_name(THIS_MODULE));
    kobject_put(addr_obj);
    if (!IS_ERR(event)){
        unregister_wide_hw_breakpoint(event);
        pr_info("[%s]: bye\n", module_name(THIS_MODULE));
        return;
    }
    pr_err("[%s]: error in unregistering\n", module_name(THIS_MODULE));
}


module_init(init_mmodule);
module_exit(exit_mmodule);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("R");
MODULE_DESCRIPTION("F*cking kernel module)");