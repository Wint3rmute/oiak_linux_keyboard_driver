#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>

#define DRIVER_VERSION ""
#define DRIVER_AUTHOR ""
#define DRIVER_DESC ""
#define DRIVER_LICENSE "GPL"

// length of konami code
#define KONAMI_LENGTH 10

MODULE_LICENSE(DRIVER_LICENSE);

// konami sequence; first from left is the last in sequence
static const unsigned char konami[KONAMI_LENGTH] = {103, 103, 108, 108, 105, 106, 105, 106, 48, 30};

static int current_konami_index = 0;

static const unsigned char keycodes[256] =
    {
        0, 0, 0, 0, 30, 48, 46, 32, 18, 33, 34, 35, 23, 36, 37, 38,
        50, 49, 24, 25, 16, 19, 31, 20, 22, 47, 17, 45, 21, 44, 2, 3,
        4, 5, 6, 7, 8, 9, 10, 11, 28, 1, 14, 15, 57, 12, 13, 26,
        27, 43, 43, 39, 40, 41, 51, 52, 53, 58, 59, 60, 61, 62, 63, 64,
        65, 66, 67, 68, 87, 88, 99, 70, 119, 110, 102, 104, 111, 107, 109, 106,
        105, 108, 103, 69, 98, 55, 74, 78, 96, 79, 80, 81, 75, 76, 77, 71,
        72, 73, 82, 83, 86, 127, 116, 117, 183, 184, 185, 186, 187, 188, 189, 190,
        191, 192, 193, 194, 134, 138, 130, 132, 128, 129, 131, 137, 133, 135, 136, 113,
        115, 114, 0, 0, 0, 121, 0, 89, 93, 124, 92, 94, 95, 0, 0, 0,
        122, 123, 90, 91, 85, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        29, 42, 56, 125, 97, 54, 100, 126, 164, 166, 165, 163, 161, 115, 114, 113,
        150, 158, 159, 128, 136, 177, 178, 176, 142, 152, 173, 140}; // standard keycodes for usb keyboard

//Defining device specific structure
struct keyboard_device_t
{
    struct usb_device *usbdev;       // Device representation
    struct input_dev *indev;         // To have representation as input  (?? input_device or input_dev ?? TO CHECK)
    struct usb_interface *interface; // Interface representation
    struct urb *irq;                 // URB (USB request block) for reading pressed key

    dma_addr_t dma_for_irq; //DMA address for irq
    unsigned char old[8];   // data received earlier from irq urb representing which keys were pressed;
                            // by comparing with the list of current pressed key, we can see which key is released

    unsigned char *new_buff; // Buffer for the irq  URB
    char physical[64];       // Physical path
    char name[128];
    // unsigned char *last[KONAMI_LENGTH]; // last pressed buttons
    bool pressed;             // defines whether the button was pressed or released
};


static int keyboard_alloc_mem(struct usb_device *dev, struct keyboard_device_t *keyboard)
{

    if (!(keyboard->irq = usb_alloc_urb(0, GFP_KERNEL)))
        return -1;

    if (!(keyboard->new_buff = usb_alloc_coherent(dev, 8, GFP_ATOMIC, &keyboard->dma_for_irq)))
        return -1;

    return 0;
}

static void keyboard_free_mem(struct usb_device *dev, struct keyboard_device_t *keyboard)
{
    usb_free_urb(keyboard->irq);
    usb_free_coherent(dev, 8, keyboard->new_buff, keyboard->dma_for_irq);
}

static void usb_keyboard_irq(struct urb *urb)
{

    //printk(KERN_INFO "OMG OMG OMG OMG OMGOMGOMGOGOG OMG OMG");

    int i,j,key;
    // new array that will hold copy of 'last' array
    struct keyboard_device_t *keyboard = urb->context;
    // specifies button status (pressed [true] or released [false])
    keyboard->pressed = !(keyboard->pressed);

    // check status of USB request block
    switch (urb->status)
    {
    case 0:
        break;

    case -ECONNRESET: // connection reset by peer
    case -ENOENT:     // no such file or directory
    case -ESHUTDOWN:  // cannot send after transport endpoint shutdown
        return;

    default:
        goto resubmit;
    }

    // loop over char (8 bits)



    for (i = 0; i < 8; i++)
    {
        // call to the input system
        // the 224 offset is there because first 4 values in keycodes[] are not used and
        // the last 24 values are for non-printable characters (Ctrl, Alt, ...)
        // ex. input_report_key(button_dev, BTN_0, inb(BUTTON_PORT) & 1);
        input_report_key(keyboard->indev, keycodes[i + 224],
                         (keyboard->new_buff[0] >> 1) & 1);
    }

  
    // translation of scancodes received from keyboard
    // into codes that are understandable for kernel
    for (i = 2; i < 8; i++)
    {
        if (keyboard->old[i] > 3 && memscan(keyboard->new_buff + 2,
                                            keyboard->old[i], 6) == keyboard->new_buff + 8)
        {
            if (keycodes[keyboard->old[i]])
	    {	input_report_key(keyboard->indev, keycodes[keyboard->old[i]], 0);
	    	printk(KERN_INFO "ehh %d", keycodes[keyboard->old[i]] );
	    if(konami[current_konami_index] == keycodes[keyboard->old[i]] ) {

		    printk(KERN_INFO "progresss %d", ++current_konami_index);
		
		    if(current_konami_index == 10) {
			printk(KERN_INFO "AK jednak zdane");
			current_konami_index = 0;
		    }
	
	    } else {
		    printk(KERN_INFO "regress"); current_konami_index = 0; }


	    }
            else
                hid_info(urb->dev,
                         "Unknown key (scancode %#x) released.\n", keyboard->old[i]);
        }

        if (keyboard->new_buff[i] > 3 && memscan(keyboard->old + 2,
                                                 keyboard->new_buff[i], 6) == keyboard->old + 8)
        {
            if (keycodes[keyboard->new_buff[i]])
                input_report_key(keyboard->indev, keycodes[keyboard->new_buff[i]], true);
            else
                hid_info(urb->dev,
                         "Unknown key (scancode %#x) pressed.\n", keyboard->new_buff[i]);
        }
    }

    // call to tell the receiver that we've sent a complete report
    input_sync(keyboard->indev);
    memcpy(keyboard->old, keyboard->new_buff, 8);

// try to resubmit urb if unable to check status
resubmit:
    i = usb_submit_urb(urb, GFP_ATOMIC);
    if (i)
    {
        hid_err(urb->dev, "cannot resubmit intr, %s-%s/input0, status %d",
                keyboard->usbdev->bus->bus_name,
                keyboard->usbdev->devpath, i);
    }

}

static void keyboard_disconnect(struct usb_interface *interface)
{
    struct keyboard_device_t *kbd_dev;
    kbd_dev = usb_get_intfdata(interface); // Reverse of usb_set_intfdata()

    //Zeroing interface data
    usb_set_intfdata(interface, NULL);
    if (kbd_dev)
    {
        usb_kill_urb(kbd_dev->irq);
        /*"usb_kill_urb()" cancels an in-progress request.
         * It is guaranteed that upon return all completion handlers will have finished
         * and the URB will be totally idle and available for reuse.*/

        input_unregister_device(kbd_dev->indev);

        // Freeing memory
        usb_free_urb(kbd_dev->irq);

        /*free memory allocated with usb_alloc_coherent during memory allocation*/
        usb_free_coherent(interface_to_usbdev(interface), 8, kbd_dev, kbd_dev->dma_for_irq);
        keyboard_free_mem(interface_to_usbdev(interface), kbd_dev);
        kfree(kbd_dev);
    }
}

static int keyboard_open(struct input_dev *dev)
{

	// converted input_dev driver to driver's model view
    struct keyboard_device_t *kbd = input_get_drvdata(dev);

    // set interrupt reques's device to current device
    kbd->irq->dev = kbd->usbdev;

    // submiting urb to usb subsystem
    if (usb_submit_urb(kbd->irq, GFP_KERNEL))
        return -EIO;

    return 0;
}

static void keyboard_close(struct input_dev *dev)
{
    // converted input_dev to driver's model view
    struct keyboard_device_t *kbd = input_get_drvdata(dev);

    // stop transferring processes
    usb_kill_urb(kbd->irq);
}



static int keyboard_probe(struct usb_interface *iface,
                          const struct usb_device_id *id)
{

    printk(KERN_INFO "PROBE PROBE PROBE PROBE PROBE PROBE PROBE PROBE PROBE PROBE");

    struct usb_device *dev = interface_to_usbdev(iface);
    struct usb_host_interface *interface;
    struct usb_endpoint_descriptor *endpoint;
    struct keyboard_device_t *keyboard_device;
    struct input_dev *inputDev;
    int i, pipe, maxpacket;
    int retval = -ENOMEM;

    interface = iface->cur_altsetting; /* FROM struct usb_interface. cur_altsetting - current altsetting.
    altsetting - array of interface structures, one for each alternate setting that may be selected. Each one includes a set of endpoint configurations. They will be in no particular order.
      */

    if (interface->desc.bNumEndpoints != 1)
        return -ENODEV;

    endpoint = &interface->endpoint[0].desc;
    if (!usb_endpoint_is_int_in(endpoint))
        return -ENODEV;

    pipe = usb_rcvintpipe(dev, endpoint->bEndpointAddress);  /* interrupt pipe, holds endpoint number */
    maxpacket = usb_maxpacket(dev, pipe, usb_pipeout(pipe)); //Maximum packet sizes for interrupt transfers

    // Allocate device specific structure
    keyboard_device = kzalloc(sizeof(struct keyboard_device_t), GFP_KERNEL);

    /* Allocating keybord representing as INPUT DEVICE */
    inputDev = input_allocate_device(); // Allocating our input device

    if (!keyboard_device || !inputDev) // Checking if allocating terminated correctly
        goto fail1;

    if (keyboard_alloc_mem(dev, keyboard_device)) // allocating memory
        goto fail2;

    keyboard_device->usbdev = dev;
    keyboard_device->indev = inputDev;
    //spin_lock_init(&kbd->leds_lock);

    if (dev->manufacturer)
        strlcpy(keyboard_device->name, dev->manufacturer, sizeof(keyboard_device->name));

    if (dev->product)
    {
        if (dev->manufacturer)
            strlcat(keyboard_device->name, " ", sizeof(keyboard_device->name));
        strlcat(keyboard_device->name, dev->product, sizeof(keyboard_device->name));
    }

    if (!strlen(keyboard_device->name))
        snprintf(keyboard_device->name, sizeof(keyboard_device->name),
                 "USB HIDBP Keyboard %04x:%04x",
                 le16_to_cpu(dev->descriptor.idVendor),
                 le16_to_cpu(dev->descriptor.idProduct));

    usb_make_path(dev, keyboard_device->physical, sizeof(keyboard_device->physical)); /* returns stable device path in the
    USB Tree (to second argument) */

    strlcat(keyboard_device->physical, "/input0", sizeof(keyboard_device->physical));

    inputDev->name = keyboard_device->name;
    inputDev->phys = keyboard_device->physical;

    usb_to_input_id(dev, &inputDev->id); /*From #include <linux/usb/input.h> adds necessary information to input device based on usb device
    (id->bustype, id->vendor, id->product, id->version) */

    inputDev->dev.parent = &iface->dev;

    input_set_drvdata(inputDev, keyboard_device); // storing device's instance for retrieval using dev_set_drvdata()

    inputDev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_LED) |
                         BIT_MASK(EV_REP); //specifying the type of event the device support   (in this case key event),



    for (i = 0; i < 255; i++)
    {
        set_bit(keycodes[i], inputDev->keybit);

        /* keybit is member of input_dev struct,
        * it is a bitmap of keys this device has */
    }
    clear_bit(0, inputDev->keybit);

    //input_dev->event = usb_kbd_event;
    inputDev->open = keyboard_open;
    inputDev->close = keyboard_close;

    usb_fill_int_urb(keyboard_device->irq, dev, pipe,
                     keyboard_device->new_buff, (maxpacket > 8 ? 8 : maxpacket),
                     usb_keyboard_irq, keyboard_device, endpoint->bInterval);

    keyboard_device->irq->transfer_dma = keyboard_device->dma_for_irq;

    keyboard_device->irq->transfer_flags |= URB_NO_TRANSFER_DMA_MAP; /*setting this flag because driver
 * provided DMA address (in previous line) */

    retval = input_register_device(keyboard_device->indev);
    if (retval)
        goto fail2;

    usb_set_intfdata(iface, keyboard_device); /* save our data pointer in this interface device */
    device_set_wakeup_enable(&dev->dev, 1);
    return 0;

fail2:
    keyboard_free_mem(dev, keyboard_device);
fail1:
    input_free_device(inputDev);
    kfree(keyboard_device);
    return retval;
}

/* table of devices that work with this driver */
static const struct usb_device_id keyboard_id_table[] = {
    {USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT,
                        USB_INTERFACE_PROTOCOL_KEYBOARD)},
    {} /* Terminating entry */
};

MODULE_DEVICE_TABLE(usb, keyboard_id_table);

static struct usb_driver keyboard_driver =
    {
        .name = "Simple keyboard driver",
        .probe = keyboard_probe,
        .disconnect = keyboard_disconnect,
        .id_table = keyboard_id_table,
};

module_usb_driver(keyboard_driver); //calling it replaces module_init and module_exit
