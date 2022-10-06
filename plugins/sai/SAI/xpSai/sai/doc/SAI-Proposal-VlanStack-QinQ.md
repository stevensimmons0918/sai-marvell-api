
Add support for Vlan Stack
-------------------------------------------------------------------------------

 Title       | Vlan Stack
-------------|-----------------------------------------------------------------
 Authors     | Edge-Core
 Status      | In review
 Type        | Standards track
 Created     | 25/02/2022
 Updated     | -
 SAI-Version | 1.9.1

-------------------------------------------------------------------------------

## Overview ##

The Vlan Stack feature includes two functions, vlan translation and QinQ.

![qinq](figures/vlan_stack/vlan_stack_QinQ_usecase.png "Figure 1: QinQ usage scenario")
__Figure 1: QinQ usage scenario__

![vlan_translate](figures/vlan_stack/vlan_stack_vlan_xlate_usecase.png "Figure 2: Vlan translation usage scenario")
__Figure 2: Vlan Translation usage scenario__

## Packet Format ##

Untagged packet, Single tagged packet and Double tagged packet

![packet_format](figures/vlan_stack/TCPIP_802.1ad_DoubleTag.png "Figure 3: Packet packet format")
__Figure 3: Packet format__

## SAI Pipeline

The packet processing pipeline is below:
![vlan_stack-pipeline](figures/vlan_stack/vlan_stack_pipeline.png "Figure 4: Vlan Stack SAI Pipeline")
__Figure 4: Vlan Stack SAI Pipeline__

## SAI Attribute

| SAI Component | SAI attribute                                  | Description                     |
| ------------- | ---------------------------------------------- | ------------------------------- |
| VLAN Stacking | SAI_VLAN_STACK_ATTR_STAGE                      | At which packet flow direction the action should take effect. Ingress or Egress    |
|               | SAI_VLAN_STACK_ATTR_ACTION                     | Include SWAP, PUSH and POP operation    |
|               | SAI_VLAN_STACK_ATTR_VLAN_APPLIED_PRI           | The "priority" of the new VLAN tag. Default value is 0xFF which will inherit the original vlan tag priority.   |
|               | SAI_VLAN_STACK_ATTR_ORIGINAL_VLAN_ID      | Has "outer" and "inner" attribute. Used to match the processing packet dot1q header. The vlan stacking rule only take effect if the dot1q header VLAN_ID match this setting. At least one attribute should be set.   |
|               | SAI_VLAN_STACK_ATTR_PORT                       | Target port ID at which the vlan stacking will take effect   |
|               | SAI_VLAN_STACK_ATTR_APPLIED_VLAN_ID            | New vlan ID for the vlan stacking. Has "outer" and "inner" attribute. User can specify where the vlan stacking rule should take effect over the packet. If the ACTION is POP, this attribute will not take effect   |

### New Data type

The new data type will be used by vlan stacking rule to match the VID of the packet by the attribute, SAI_VLAN_STACK_ATTR_ORIGINAL_VLAN_ID.
If set the outer to 10 and set the inner to 0, it means finding the packet which has outermost VID=10.
If set the outer to 0 and set the inner to 10, it means finding the packet which has double tag and its inner tag VID=10.
If set the outer to 10 and set the inner to 20, it means finding the packet which has double tag and its outer VID=10 (SVID), inner VID=20 (CVID).

The data type will also be used to provide the new dot1q header setting after the vlan stacking rule by the attribute, SAI_VLAN_STACK_ATTR_APPLIED_VLAN_ID.
If the action is PUSH, the value of the "outer" will be used to create an outer dot1q header on the packet.
If the action is SWAP, the original dot1q header will be replaced with "outer", "inner" or both. Depends on the attribute setting.


```
/**
 * @brief Attribute data for vlan stacking rule
 * 
 * If attribute value is 0, means the attribute is invalid.
 */
typedef struct _sai_vlan_stacking_vid_t
{
    /** Vlan ID at the inner dot1q position */
    uint16_t inner;

    /** Vlan ID at the outer dot1q position */
    uint16_t outer;
} sai_vlan_stacking_vid_t;
```

## Use cases

### Ingress/Add outer tag based on outer tag
Add an outer dot1q header with VID=100 for those packets with outer VID=10 at Ethernet10

#### Usage

```
sai_object_id_t vlan_stacking_oid;
sai_attribute_t attr;
vector<sai_attribute_t> vlan_stacking_entry_attrs;

attr.id = SAI_VLAN_STACK_ATTR_STAGE;
attr.value.s32 = SAI_VLAN_STACK_STAGE_INGRESS;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_ACTION;
attr.value.s32 = SAI_VLAN_STACK_ACTION_PUSH;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_ORIGINAL_VLAN_ID;
attr.value.vlanstackingvid.outer = 10;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_PORT;
attr.value.oid = 10;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_APPLIED_VLAN_ID;
attr.value.vlanstackingvid.outer = 100;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_VLAN_APPLIED_PRI;
attr.value.u8 = 0;
vlan_stacking_entry_attrs.push(attr);

sai_vlan_api->create_vlan_stack(&vlan_stacking_oid, gSwitchId, (uint32_t)vlan_stacking_entry_attrs.size(), vlan_stacking_entry_attrs.data());
```

#### Behavior

![vlan_stack-ingress-push-outer](figures/vlan_stack/ingress_push_outer.png "Table 1: Ingress Push")
__Table 1: Vlan Stack Ingress Push Outer__

### Ingress/Add outer tag based on inner tag
Add an outer dot1q header with VID=100 for those packets with inner dot1q VID=10 at Ethernet10

#### Usage

```
sai_object_id_t vlan_stacking_oid;
sai_attribute_t attr;
vector<sai_attribute_t> vlan_stacking_entry_attrs;

attr.id = SAI_VLAN_STACK_ATTR_STAGE;
attr.value.s32 = SAI_VLAN_STACK_STAGE_INGRESS;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_ACTION;
attr.value.s32 = SAI_VLAN_STACK_ACTION_PUSH;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_ORIGINAL_VLAN_ID;
attr.value.vlanstackingvid.inner = 10;
attr.value.vlanstackingvid.outer = 0;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_PORT;
attr.value.oid = 10;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_APPLIED_VLAN_ID;
attr.value.vlanstackingvid.inner = 0;
attr.value.vlanstackingvid.outer = 100;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_VLAN_APPLIED_PRI;
attr.value.u8 = 0;
vlan_stacking_entry_attrs.push(attr);

sai_vlan_api->create_vlan_stack(&vlan_stacking_oid, gSwitchId, (uint32_t)vlan_stacking_entry_attrs.size(), vlan_stacking_entry_attrs.data());
```

#### Behavior

![vlan_stack-ingress-push-inner](figures/vlan_stack/ingress_push_inner.png "Table 2: Ingress Push")
__Table 2: Vlan Stack Ingress Push Inner__

### Egress/Remove outer tag based on outer tag
Remove outer dot1q header for those packets with original outer dot1q header VID=100 at Ethernet20

#### Usage

```
sai_object_id_t vlan_stacking_oid;
sai_attribute_t attr;
vector<sai_attribute_t> vlan_stacking_entry_attrs;

attr.id = SAI_VLAN_STACK_ATTR_STAGE;
attr.value.s32 = SAI_VLAN_STACK_STAGE_EGRESS;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_ACTION;
attr.value.s32 = SAI_VLAN_STACK_ACTION_POP;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_ORIGINAL_VLAN_ID;
attr.value.vlanstackingvid.outer = 100;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_PORT;
attr.value.oid = 20;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_VLAN_APPLIED_PRI;
attr.value.u8 = 0;
vlan_stacking_entry_attrs.push(attr);

sai_vlan_api->create_vlan_stack(&vlan_stacking_oid, gSwitchId, (uint32_t)vlan_stacking_entry_attrs.size(), vlan_stacking_entry_attrs.data());
```

#### Behavior

![vlan_stack-egress-pop-outer](figures/vlan_stack/egress_pop_outer.png "Table 3: Egress Pop Outer")
__Table 3: Vlan Stack Egress Pop Outer__

### Ingress/Swap inner tag based on inner tag
Change inner dot1q header to VID=100 for those packets with original inner dot1q header VID=10 when they flow into Ethernet10

#### Usage

```
sai_object_id_t vlan_stacking_oid;
sai_attribute_t attr;
vector<sai_attribute_t> vlan_stacking_entry_attrs;

attr.id = SAI_VLAN_STACK_ATTR_STAGE;
attr.value.s32 = SAI_VLAN_STACK_STAGE_INGRESS;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_ACTION;
attr.value.s32 = SAI_VLAN_STACK_ACTION_SWAP;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_ORIGINAL_VLAN_ID;
attr.value.vlanstackingvid.inner = 10;
attr.value.vlanstackingvid.outer = 0;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_PORT;
attr.value.oid = 10;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_APPLIED_VLAN_ID;
attr.value.vlanstackingvid.inner = 100;
attr.value.vlanstackingvid.outer = 0;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_VLAN_APPLIED_PRI;
attr.value.u8 = 0;
vlan_stacking_entry_attrs.push(attr);

sai_vlan_api->create_vlan_stack(&vlan_stacking_oid, gSwitchId, (uint32_t)vlan_stacking_entry_attrs.size(), vlan_stacking_entry_attrs.data());
```

#### Behavior

![vlan_stack-ingress-swap-inner](figures/vlan_stack/ingress_swap_inner.png "Table 4: Ingress Swap Inner")
__Table 4: Vlan Stack Ingress Swap Inner__

### Ingress/Swap outer tag based on outer tag
Change outer dot1q header to VID=100 for those packets with original outer dot1q header VID=10 when they flow into Ethernet10

#### Usage

```
sai_object_id_t vlan_stacking_oid;
sai_attribute_t attr;
vector<sai_attribute_t> vlan_stacking_entry_attrs;

attr.id = SAI_VLAN_STACK_ATTR_STAGE;
attr.value.s32 = SAI_VLAN_STACK_STAGE_INGRESS;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_ACTION;
attr.value.s32 = SAI_VLAN_STACK_ACTION_SWAP;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_ORIGINAL_VLAN_ID;
attr.value.vlanstackingvid.inner = 0;
attr.value.vlanstackingvid.outer = 10;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_PORT;
attr.value.oid = 10;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_APPLIED_VLAN_ID;
attr.value.vlanstackingvid.inner = 0;
attr.value.vlanstackingvid.outer = 100;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_VLAN_APPLIED_PRI;
attr.value.u8 = 0;
vlan_stacking_entry_attrs.push(attr);

sai_vlan_api->create_vlan_stack(&vlan_stacking_oid, gSwitchId, (uint32_t)vlan_stacking_entry_attrs.size(), vlan_stacking_entry_attrs.data());
```

#### Behavior

![vlan_stack-ingress-swap-outer](figures/vlan_stack/ingress_swap_outer.png "Table 5: Ingress Swap Outer")
__Table 5: Vlan Stack Ingress Swap Outer__

### Egress/Swap inner tag based on inner tag
Change inner dot1q header to VID=10 for those packets with original inner dot1q header VID=100 when they flow away from Ethernet20

#### Usage

```
sai_object_id_t vlan_stacking_oid;
sai_attribute_t attr;
vector<sai_attribute_t> vlan_stacking_entry_attrs;

attr.id = SAI_VLAN_STACK_ATTR_STAGE;
attr.value.s32 = SAI_VLAN_STACK_STAGE_EGRESS;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_ACTION;
attr.value.s32 = SAI_VLAN_STACK_ACTION_SWAP;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_ORIGINAL_VLAN_ID;
attr.value.vlanstackingvid.inner = 100;
attr.value.vlanstackingvid.outer = 0;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_PORT;
attr.value.oid = 20;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_APPLIED_VLAN_ID;
attr.value.vlanstackingvid.inner = 10;
attr.value.vlanstackingvid.outer = 0;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_VLAN_APPLIED_PRI;
attr.value.u8 = 0;
vlan_stacking_entry_attrs.push(attr);

sai_vlan_api->create_vlan_stack(&vlan_stacking_oid, gSwitchId, (uint32_t)vlan_stacking_entry_attrs.size(), vlan_stacking_entry_attrs.data());
```

#### Behavior

![vlan_stack-egress-swap-inner](figures/vlan_stack/egress_swap_inner.png "Table 6: Egress Swap Inner")
__Table 6: Vlan Stack Egress Swap Inner__

### Egress/Swap outer tag based on outer tag
Change outer dot1q header to VID=10 for those packets with original outer dot1q header VID=100 when they flow away from Ethernet20

#### Usage

```
sai_object_id_t vlan_stacking_oid;
sai_attribute_t attr;
vector<sai_attribute_t> vlan_stacking_entry_attrs;

attr.id = SAI_VLAN_STACK_ATTR_STAGE;
attr.value.s32 = SAI_VLAN_STACK_STAGE_EGRESS;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_ACTION;
attr.value.s32 = SAI_VLAN_STACK_ACTION_SWAP;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_ORIGINAL_VLAN_ID;
attr.value.vlanstackingvid.inner = 0;
attr.value.vlanstackingvid.outer = 100;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_PORT;
attr.value.oid = 20;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_APPLIED_VLAN_ID;
attr.value.vlanstackingvid.inner = 0;
attr.value.vlanstackingvid.outer = 10;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_VLAN_APPLIED_PRI;
attr.value.u8 = 0;
vlan_stacking_entry_attrs.push(attr);

sai_vlan_api->create_vlan_stack(&vlan_stacking_oid, gSwitchId, (uint32_t)vlan_stacking_entry_attrs.size(), vlan_stacking_entry_attrs.data());
```

#### Behavior

![vlan_stack-egress-swap-outer](figures/vlan_stack/egress_swap_outer.png "Table 7: Egress Swap Outer")
__Table 7: Vlan Stack Egress Swap Outer__

### Ingress/Swap on double tag from single tag
Change the incoming packet with outer dot1q header VID=200 to a double tag packet with outer VID=100 and inner VID=10 at port Ethernet10

#### Usage

```
sai_object_id_t vlan_stacking_oid;
sai_attribute_t attr;
vector<sai_attribute_t> vlan_stacking_entry_attrs;

attr.id = SAI_VLAN_STACK_ATTR_STAGE;
attr.value.s32 = SAI_VLAN_STACK_STAGE_INGRESS;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_ACTION;
attr.value.s32 = SAI_VLAN_STACK_ACTION_SWAP;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_ORIGINAL_VLAN_ID;
attr.value.vlanstackingvid.inner = 0;
attr.value.vlanstackingvid.outer = 200;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_PORT;
attr.value.oid = 10;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_APPLIED_VLAN_ID;
attr.value.vlanstackingvid.inner = 10;
attr.value.vlanstackingvid.outer = 100;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_VLAN_APPLIED_PRI;
attr.value.u8 = 0;
vlan_stacking_entry_attrs.push(attr);

sai_vlan_api->create_vlan_stack(&vlan_stacking_oid, gSwitchId, (uint32_t)vlan_stacking_entry_attrs.size(), vlan_stacking_entry_attrs.data());
```

#### Behavior

![vlan_stack-ingress-swap-single-double](figures/vlan_stack/ingress_swap_single_double.png "Table 8: Ingress Swap Single Double")
__Table 8: Vlan Stack Ingress Swap Single to Double__

### Egress/Swap on single tag from double tag
Change the outgoing double tag packet with outer VID=100 and inner VID=10 to a single tag packet with VID=200 at port Ethernet20

#### Usage

```
sai_object_id_t vlan_stacking_oid;
sai_attribute_t attr;
vector<sai_attribute_t> vlan_stacking_entry_attrs;

attr.id = SAI_VLAN_STACK_ATTR_STAGE;
attr.value.s32 = SAI_VLAN_STACK_STAGE_EGRESS;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_ACTION;
attr.value.s32 = SAI_VLAN_STACK_ACTION_SWAP;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_ORIGINAL_VLAN_ID;
attr.value.vlanstackingvid.inner = 10;
attr.value.vlanstackingvid.outer = 100;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_PORT;
attr.value.oid = 20;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_APPLIED_VLAN_ID;
attr.value.vlanstackingvid.inner = 0;
attr.value.vlanstackingvid.outer = 200;
vlan_stacking_entry_attrs.push(attr);

attr.id = SAI_VLAN_STACK_ATTR_VLAN_APPLIED_PRI;
attr.value.u8 = 0;
vlan_stacking_entry_attrs.push(attr);

sai_vlan_api->create_vlan_stack(&vlan_stacking_oid, gSwitchId, (uint32_t)vlan_stacking_entry_attrs.size(), vlan_stacking_entry_attrs.data());
```

#### Behavior

![vlan_stack-egress-swap-double-single](figures/vlan_stack/egress_swap_double_single.png "Table 9: Egress Swap Double Single")
__Table 9: Vlan Stack Ingress Swap Double to Single__