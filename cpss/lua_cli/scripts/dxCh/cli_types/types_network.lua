--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* types_network.lua
--*
--* DESCRIPTION:
--*       network types
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

CLI_type_dict["cpu_code_number"] = {
    checker = CLI_check_param_number,
    min=0,
    max=255,
    complete = CLI_complete_param_number,
    help="CPU Code number. Described in functional specification of device."
}

CLI_type_dict["cpu_code_range"] = {
    checker = CLI_check_param_number_range,
    min=0,
    max=255,
    complete = CLI_complete_param_number_range,
    help = "Range of CPU Code numbers from functional specification"
}

CLI_type_dict["cpu_code_enum"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "CPU Code Name",
    enum = {
        ["control"] = {value = 1, help = "control"},
        ["un-known-uc"] = {value = 10, help = "un-known-uc"},
        ["un-registerd-mc"] = {value = 11, help = "un-registerd-mc"},
        ["control-bpdu"] = {value = 16, help = "control-bpdu"},
        ["control-dest-mac-trap"] = {value = 17, help = "control-dest-mac-trap"},
        ["control-src-mac-trap"] = {value = 18, help = "control-src-mac-trap"},
        ["control-src-dst-mac-trap"] = {value = 19, help = "control-src-dst-mac-trap"},
        ["control-mac-range-trap"] = {value = 20, help = "control-mac-range-trap"},
        ["rx-sniffer-trap"] = {value = 21, help = "rx-sniffer-trap"},
        ["intervention-arp"] = {value = 32, help = "intervention-arp"},
        ["intervention-igmp"] = {value = 33, help = "intervention-igmp"},
        ["intervention-sa"] = {value = 34, help = "intervention-sa"},
        ["intervention-da"] = {value = 35, help = "intervention-da"},
        ["intervention-sa-da"] = {value = 36, help = "intervention-sa-da"},
        ["intervention-port-lock"] = {value = 37, help = "intervention-port-lock"},
        ["exp-tnl-bad-ipv4-hdr"] = {value = 38, help = "exp-tnl-bad-ipv4-hdr"},
        ["exp-tnl-bad-vlan"] = {value = 39, help = "exp-tnl-bad-vlan"},
        ["mll-rpf-trap"] = {value = 120, help = "mll-rpf-trap"},
        ["reserved-sip-trap"] = {value = 128, help = "reserved-sip-trap"},
        ["internal-sip-trap"] = {value = 129, help = "internal-sip-trap"},
        ["sip-spoof-trap"] = {value = 130, help = "sip-spoof-trap"},
        ["def-key-trap"] = {value = 132, help = "def-key-trap"},
        ["ip-class-trap"] = {value = 133, help = "ip-class-trap"},
        ["class-key-trap"] = {value = 134, help = "class-key-trap"},
        ["tcp-rst-fin-trap"] = {value = 135, help = "tcp-rst-fin-trap"},
        ["class-key-mirror"] = {value = 136, help = "class-key-mirror"},
        ["trap-by-default-entry0"] = {value = 137, help = "trap-by-default-entry0"},
        ["trap-by-default-entry1"] = {value = 138, help = "trap-by-default-entry1"},
        ["trap-ud-invalid"] = {value = 139, help = "trap-ud-invalid"},
        ["reserved-dip-trap"] = {value = 144, help = "reserved-dip-trap"},
        ["mc-boundary-trap"] = {value = 145, help = "mc-boundary-trap"},
        ["internal-dip"] = {value = 146, help = "internal-dip"},
        ["ip-zero-ttl-trap"] = {value = 147, help = "ip-zero-ttl-trap"},
        ["bad-ip-hdr-checksum"] = {value = 148, help = "bad-ip-hdr-checksum"},
        ["rpf-check-failed"] = {value = 149, help = "rpf-check-failed"},
        ["options-in-ip-hdr"] = {value = 150, help = "options-in-ip-hdr"},
        ["end-of-ip-tunnel"] = {value = 151, help = "end-of-ip-tunnel"},
        ["bad-tunnel-hdr"] = {value = 152, help = "bad-tunnel-hdr"},
        ["ip-hdr-error"] = {value = 153, help = "ip-hdr-error"},
        ["route-entry-trap"] = {value = 154, help = "route-entry-trap"},
        ["dip-check-error"] = {value = 155, help = "dip-check-error"},
        ["illegal-dip"] = {value = 156, help = "illegal-dip"},
        ["policy-trap"] = {value = 157, help = "policy-trap"},
        ["default-route-trap"] = {value = 158, help = "default-route-trap"},
        ["ip-mtu-exceed"] = {value = 161, help = "ip-mtu-exceed"},
        ["mpls-mtu-exceed"] = {value = 162, help = "mpls-mtu-exceed"},
        ["class-mtu-exceed"] = {value = 163, help = "class-mtu-exceed"},
        ["mpls-zero-ttl-trap"] = {value = 171, help = "mpls-zero-ttl-trap"},
        ["nhlfe-entry-trap"] = {value = 172, help = "nhlfe-entry-trap"},
        ["illegal-pop"] = {value = 173, help = "illegal-pop"},
        ["invalid-mpls-if"] = {value = 174, help = "invalid-mpls-if"},
        ["ipv6-icmp-mld"] = {value = 175, help = "ipv6-icmp-mld"},
        ["ipv6-igmp"] = {value = 176, help = "ipv6-igmp"},
        ["ipv6-mc-pcl"] = {value = 186, help = "ipv6-mc-pcl"},
        ["ipv6-hop-by-hop"] = {value = 187, help = "ipv6-hop-by-hop"},
        ["ipv6-ext-header"] = {value = 188, help = "ipv6-ext-header"},
        ["ipv6-bad-header"] = {value = 189, help = "ipv6-bad-header"},
        ["ipv6-illigal-dip"] = {value = 190, help = "ipv6-illigal-dip"},
        ["ipv6-dip-error"] = {value = 191, help = "ipv6-dip-error"},
        ["ipv6-route-trap"] = {value = 192, help = "ipv6-route-trap"},
        ["ipv6-router-rpf"] = {value = 193, help = "ipv6-router-rpf"},
        ["ipv6-scop-fail"] = {value = 194, help = "ipv6-scop-fail"},
        ["ipv6-ttl-trap"] = {value = 195, help = "ipv6-ttl-trap"},
        ["ipv6-default-route"] = {value = 196, help = "ipv6-default-route"},
        ["mirror-pcl-rst-fin"] = {value = 201, help = "mirror-pcl-rst-fin"},
        ["mirror-pcl-pce"] = {value = 202, help = "mirror-pcl-pce"},
        ["mirror-bridge"] = {value = 203, help = "mirror-bridge"},
        ["mirror-mld"] = {value = 204, help = "mirror-mld"},
        ["mirror-igmp"] = {value = 205, help = "mirror-igmp"},
        ["mirror-ripv1"] = {value = 206, help = "mirror-ripv1"},
        ["mirror-ttl"] = {value = 207, help = "mirror-ttl"},
        ["mirror-rpf"] = {value = 208, help = "mirror-rpf"},
        ["mirror-ip-options"] = {value = 209, help = "mirror-ip-options"},
        ["mirror-route"] = {value = 210, help = "mirror-route"},
        ["mirror-icmp"] = {value = 211, help = "mirror-icmp"},
        ["mirror-hop-by-hop"] = {value = 212, help = "mirror-hop-by-hop"},
        ["mirror-ext-header"] = {value = 213, help = "mirror-ext-header"},
        ["mirror-header-error"] = {value = 214, help = "mirror-header-error"},
        ["mirror-illigal-ip"] = {value = 215, help = "mirror-illigal-ip"},
        ["mirror-special-dip"] = {value = 216, help = "mirror-special-dip"},
        ["mirror-ip-scop"] = {value = 217, help = "mirror-ip-scop"},
        ["mirror-ip-mtu"] = {value = 218, help = "mirror-ip-mtu"},
        ["mirror-inlif"] = {value = 219, help = "mirror-inlif"},
        ["mirror-pcl-mtu"] = {value = 220, help = "mirror-pcl-mtu"},
        ["mirror-default-route"] = {value = 221, help = "mirror-default-route"},
        ["mirror-mc-route1"] = {value = 222, help = "mirror-mc-route1"},
        ["mirror-mc-route2"] = {value = 223, help = "mirror-mc-route2"},
        ["mirror-mc-route3"] = {value = 224, help = "mirror-mc-route3"},
        ["mirror-ipv4-bc"] = {value = 225, help = "mirror-ipv4-bc"},
        ["eth-bridged-llt"] = {value = 248, help = "eth-bridged-llt"},
        ["ipv4-routed-llt"] = {value = 249, help = "ipv4-routed-llt"},
        ["uc-mpls-llt"] = {value = 250, help = "uc-mpls-llt"},
        ["mc-mpls-llt"] = {value = 251, help = "mc-mpls-llt"},
        ["ipv6-routed-llt"] = {value = 252, help = "ipv6-routed-llt"},
        ["l2ce-llt"] = {value = 253, help = "l2ce-llt"},
        ["egress-mirror-to-cpu"] = {value = 254, help = "egress-mirror-to-cpu"},
        ["unknown-uc"] = {value = 261, help = "unknown-uc"},
        ["unreg-mc"] = {value = 262, help = "unreg-mc"},
        ["lock-port-mirror"] = {value = 271, help = "lock-port-mirror"},
        ["ieee-rsrvd-multicast-addr"] = {value = 300, help = "ieee-rsrvd-multicast-addr"},
        ["ipv6-icmp-packet"] = {value = 301, help = "ipv6-icmp-packet"},
        ["mirror-ipv4-ripv1-to-cpu"] = {value = 302, help = "mirror-ipv4-ripv1-to-cpu"},
        ["ipv6-neighbor-solicitation"] = {value = 303, help = "ipv6-neighbor-solicitation"},
        ["ipv4-broadcast-packet"] = {value = 304, help = "ipv4-broadcast-packet"},
        ["non-ipv4-broadcast-packet"] = {value = 305, help = "non-ipv4-broadcast-packet"},
        ["cisco-multicast-mac-range"] = {value = 306, help = "cisco-multicast-mac-range"},
        ["unregistered-multicast"] = {value = 307, help = "unregistered-multicast"},
        ["ipv4-unregistered-multicast"] = {value = 308, help = "ipv4-unregistered-multicast"},
        ["ipv6-unregistered-multicast"] = {value = 309, help = "ipv6-unregistered-multicast"},
        ["unknown-unicast"] = {value = 310, help = "unknown-unicast"},
        ["udp-bc-mirror-trap0"] = {value = 322, help = "udp-bc-mirror-trap0"},
        ["udp-bc-mirror-trap1"] = {value = 323, help = "udp-bc-mirror-trap1"},
        ["udp-bc-mirror-trap2"] = {value = 324, help = "udp-bc-mirror-trap2"},
        ["udp-bc-mirror-trap3"] = {value = 325, help = "udp-bc-mirror-trap3"},
        ["ieee-rsrvd-multicast-addr-1"] = {value = 326, help = "ieee-rsrvd-multicast-addr-1"},
        ["ieee-rsrvd-multicast-addr-2"] = {value = 327, help = "ieee-rsrvd-multicast-addr-2"},
        ["ieee-rsrvd-multicast-addr-3"] = {value = 328, help = "ieee-rsrvd-multicast-addr-3"},
        ["bridged-packet-forward"] = {value = 400, help = "bridged-packet-forward"},
        ["ingress-mirrored-to-anlyzer"] = {value = 401, help = "ingress-mirrored-to-anlyzer"},
        ["egress-mirrored-to-anlyzer"] = {value = 402, help = "egress-mirrored-to-anlyzer"},
        ["mail-from-neighbor-cpu"] = {value = 403, help = "mail-from-neighbor-cpu"},
        ["cpu-to-cpu"] = {value = 404, help = "cpu-to-cpu"},
        ["egress-sampled"] = {value = 405, help = "egress-sampled"},
        ["ingress-sampled"] = {value = 406, help = "ingress-sampled"},
        ["invalid-pcl-key-trap"] = {value = 409, help = "invalid-pcl-key-trap"},
        ["ipv4-ipv6-link-local-mc-dip-trp-mrr"] = {value = 410, help = "ipv4-ipv6-link-local-mc-dip-trp-mrr"},
        ["packet-to-virtual-router-port"] = {value = 411, help = "packet-to-virtual-router-port"},
        ["mirror-ipv4-uc-icmp-redirect"] = {value = 412, help = "mirror-ipv4-uc-icmp-redirect"},
        ["mirror-ipv6-uc-icmp-redirect"] = {value = 413, help = "mirror-ipv6-uc-icmp-redirect"},
        ["routed-packet-forward"] = {value = 414, help = "routed-packet-forward"},
        ["ip-dip-da-mismatch"] = {value = 415, help = "ip-dip-da-mismatch"},
        ["ip-uc-sip-sa-mismatch"] = {value = 416, help = "ip-uc-sip-sa-mismatch"},
        ["ipv4-uc-route1-trap"] = {value = 417, help = "ipv4-uc-route1-trap"},
        ["ipv4-uc-route2-trap"] = {value = 418, help = "ipv4-uc-route2-trap"},
        ["ipv4-uc-route3-trap"] = {value = 419, help = "ipv4-uc-route3-trap"},
        ["ipv4-mc-route0-trap"] = {value = 420, help = "ipv4-mc-route0-trap"},
        ["ipv4-mc-route1-trap"] = {value = 421, help = "ipv4-mc-route1-trap"},
        ["ipv4-mc-route2-trap"] = {value = 422, help = "ipv4-mc-route2-trap"},
        ["ipv4-mc-route3-trap"] = {value = 423, help = "ipv4-mc-route3-trap"},
        ["ipv6-uc-route1-trap"] = {value = 424, help = "ipv6-uc-route1-trap"},
        ["ipv6-uc-route2-trap"] = {value = 425, help = "ipv6-uc-route2-trap"},
        ["ipv6-uc-route3-trap"] = {value = 426, help = "ipv6-uc-route3-trap"},
        ["ipv6-mc-route0-trap"] = {value = 427, help = "ipv6-mc-route0-trap"},
        ["ipv6-mc-route1-trap"] = {value = 428, help = "ipv6-mc-route1-trap"},
        ["ipv6-mc-route2-trap"] = {value = 430, help = "ipv6-mc-route2-trap"},
        ["ipv6-mc-route3-trap"] = {value = 431, help = "ipv6-mc-route3-trap"},
        ["ip-uc-rpf-fail"] = {value = 432, help = "ip-uc-rpf-fail"},
        ["arp-bc-to-me"] = {value = 433, help = "arp-bc-to-me"},
        ["ipv4-ipv6-link-local-mc-dip-trp-mrr-1"] = {value = 434, help = "ipv4-ipv6-link-local-mc-dip-trp-mrr-1"},
        ["ipv4-ipv6-link-local-mc-dip-trp-mrr-2"] = {value = 435, help = "ipv4-ipv6-link-local-mc-dip-trp-mrr-2"},
        ["ipv4-ipv6-link-local-mc-dip-trp-mrr-3"] = {value = 436, help = "ipv4-ipv6-link-local-mc-dip-trp-mrr-3"},
        ["sec-auto-learn-unk-src-trap"] = {value = 437, help = "sec-auto-learn-unk-src-trap"},
        ["ipv4-tt-header-error"] = {value = 438, help = "ipv4-tt-header-error"},
        ["ipv4-tt-option-frag-error"] = {value = 439, help = "ipv4-tt-option-frag-error"},
        ["ipv4-tt-unsup-gre-error"] = {value = 440, help = "ipv4-tt-unsup-gre-error"},
        ["arp-reply-to-me"] = {value = 441, help = "arp-reply-to-me"},
        ["cpu-to-all-cpus"] = {value = 442, help = "cpu-to-all-cpus"},
        ["tcp-syn-to-cpu"] = {value = 443, help = "tcp-syn-to-cpu"},
        ["mc-bridged-packet-forward"] = {value = 445, help = "mc-bridged-packet-forward"},
        ["mc-routed-packet-forward"] = {value = 446, help = "mc-routed-packet-forward"},
        ["l2vpn-packet-forward"] = {value = 447, help = "l2vpn-packet-forward"},
        ["mpls-packet-forward"] = {value = 448, help = "mpls-packet-forward"},
        ["arp-bc-egress-mirror"] = {value = 451, help = "arp-bc-egress-mirror"},
        ["vpls-unregistered-mc-egress-filter"] = {value = 452, help = "vpls-unregistered-mc-egress-filter"},
        ["vpls-unkwonw-uc-egress-filter"] = {value = 453, help = "vpls-unkwonw-uc-egress-filter"},
        ["vpls-bc-egress-filter"] = {value = 454, help = "vpls-bc-egress-filter"},
        ["mc-cpu-to-cpu"] = {value = 455, help = "mc-cpu-to-cpu"},
        ["pcl-mirrored-to-analyzer"] = {value = 456, help = "pcl-mirrored-to-analyzer"},
        ["vpls-untagged-mru-filter"] = {value = 457, help = "vpls-untagged-mru-filter"},
        ["tt-mpls-header-check"] = {value = 458, help = "tt-mpls-header-check"},
        ["tt-mpls-ttl-exceed"] = {value = 459, help = "tt-mpls-ttl-exceed"},
        ["tti-mirror"] = {value = 460, help = "tti-mirror"},
        ["mpls-src-filtering"] = {value = 461, help = "mpls-src-filtering"},
        ["ipv4-ttl1-exceeded"] = {value = 462, help = "ipv4-ttl1-exceeded"},
        ["ipv6-hoplimit1-exceed"] = {value = 463, help = "ipv6-hoplimit1-exceed"},
        ["oam-pdu-trap"] = {value = 464, help = "oam-pdu-trap"},
        ["ipcl-mirror"] = {value = 465, help = "ipcl-mirror"},
        ["mpls-ttl1-exceeded"] = {value = 468, help = "mpls-ttl1-exceeded"},
        ["mpls-route-entry-1"] = {value = 470, help = "mpls-route-entry-1"},
        ["mpls-route-entry-2"] = {value = 471, help = "mpls-route-entry-2"},
        ["mpls-route-entry-3"] = {value = 472, help = "mpls-route-entry-3"},
        ["ptp-header-error"] = {value = 473, help = "ptp-header-error"},
        ["ipv4-6-sip-filtering"] = {value = 474, help = "ipv4-6-sip-filtering"},
        ["ipv4-6-sip-is-zero"] = {value = 475, help = "ipv4-6-sip-is-zero"},
        ["access-matrix"] = {value = 476, help = "access-matrix"},
        ["fcoe-dip-lookup-not-found-or-fcoe-exception"] = {value = 477, help = "fcoe-dip-lookup-not-found-or-fcoe-exception"},
        ["fcoe-sip-not-found"] = {value = 478, help = "fcoe-sip-not-found"},
        ["ip-mc-route-bidir-rpf-fail"] = {value = 479, help = "ip-mc-route-bidir-rpf-fail"},
        ["met-red-drop-code"] = {value = 480, help = "met-red-drop-code"},
        ["ecc-drop-code"] = {value = 481, help = "ecc-drop-code"},
        ["bad-analyzer-index-drop-error"] = {value = 482, help = "bad-analyzer-index-drop-error"},
        ["rxdma-drop"] = {value = 483, help = "rxdma-drop"},
        ["target-null-port"] = {value = 484, help = "target-null-port"},
        ["target-eport-mtu-exceeded-or-rx-protection-switching-or-ingress-forwarding-restrictions"] = {value = 485, help = "target-eport-mtu-exceeded-or-rx-protection-switching-or-ingress-forwarding-restrictions"},
        ["untagged-oversize-drop-code"] = {value = 486, help = "untagged-oversize-drop-code"},
        ["ipv6-tt-unsup-gre-error"] = {value = 487, help = "ipv6-tt-unsup-gre-error"},
        ["ccfc-pkt-drop"] = {value = 488, help = "ccfc-pkt-drop"},
        ["bad-fc-pkt-drop"] = {value = 489, help = "bad-fc-pkt-drop"},
        ["first-user-defined"] = {value = 500, help = "first-user-defined"},
        ["first-unknown-hw-cpu-code"] = {value = 1024, help = "first-unknown-hw-cpu-code"},
   }
}

