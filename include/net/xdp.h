/* include/net/xdp.h
 *
 * Copyright (c) 2017 Jesper Dangaard Brouer, Red Hat Inc.
 * Released under terms in GPL version 2.  See COPYING.
 */
#ifndef __LINUX_NET_XDP_H__
#define __LINUX_NET_XDP_H__

/**
 * DOC: XDP RX-queue information
 *
 * The XDP RX-queue info (xdp_rxq_info) is associated with the driver
 * level RX-ring queues.  It is information that is specific to how
 * the driver have configured a given RX-ring queue.
 *
 * Each xdp_buff frame received in the driver carry a (pointer)
 * reference to this xdp_rxq_info structure.  This provides the XDP
 * data-path read-access to RX-info for both kernel and bpf-side
 * (limited subset).
 *
 * For now, direct access is only safe while running in NAPI/softirq
 * context.  Contents is read-mostly and must not be updated during
 * driver NAPI/softirq poll.
 *
 * The driver usage API is a register and unregister API.
 *
 * The struct is not directly tied to the XDP prog.  A new XDP prog
 * can be attached as long as it doesn't change the underlying
 * RX-ring.  If the RX-ring does change significantly, the NIC driver
 * naturally need to stop the RX-ring before purging and reallocating
 * memory.  In that process the driver MUST call unregistor (which
 * also apply for driver shutdown and unload).  The register API is
 * also mandatory during RX-ring setup.
 */

enum xdp_mem_type {
	MEM_TYPE_PAGE_SHARED = 0, /* Split-page refcnt based model */
	MEM_TYPE_PAGE_ORDER0,     /* Orig XDP full page model */
	MEM_TYPE_PAGE_POOL,
	MEM_TYPE_ZERO_COPY,
	MEM_TYPE_MAX,
};

/* XDP flags for ndo_xdp_xmit */
#define XDP_XMIT_FLAGS_NONE	0U
#define XDP_XMIT_FLUSH		(1U << 0)	/* doorbell signal consumer */
#define XDP_XMIT_FLAGS_MASK	XDP_XMIT_FLUSH

struct xdp_mem_info {
	u32 type; /* enum xdp_mem_type, but known size type */
};

struct page_pool;

struct zero_copy_allocator {
	void (*free)(struct zero_copy_allocator *zca, unsigned long handle);
};

struct xdp_rxq_info {
	struct net_device *dev;
	u32 queue_index;
	u32 reg_state;
	struct xdp_mem_info mem;
} ____cacheline_aligned; /* perf critical, avoid false-sharing */

struct xdp_buff {
	void *data;
	void *data_end;
	void *data_meta;
	void *data_hard_start;
	unsigned long handle;
	struct xdp_rxq_info *rxq;
};

struct xdp_frame {
	void *data;
	u16 len;
	u16 headroom;
	u16 metasize;
	/* Lifetime of xdp_rxq_info is limited to NAPI/enqueue time,
	 * while mem info is valid on remote CPU.
	 */
	struct xdp_mem_info mem;
};

/* Convert xdp_buff to xdp_frame */
static inline
struct xdp_frame *convert_to_xdp_frame(struct xdp_buff *xdp)
{
	return NULL;
}

static inline
void xdp_return_frame(struct xdp_frame *xdpf)
{
	return;
}

static inline
void xdp_return_frame_rx_napi(struct xdp_frame *xdpf)
{
	return;
}

int xdp_rxq_info_reg(struct xdp_rxq_info *xdp_rxq,
		     struct net_device *dev, u32 queue_index);
void xdp_rxq_info_unreg(struct xdp_rxq_info *xdp_rxq);
void xdp_rxq_info_unused(struct xdp_rxq_info *xdp_rxq);
bool xdp_rxq_info_is_reg(struct xdp_rxq_info *xdp_rxq);
int xdp_rxq_info_reg_mem_model(struct xdp_rxq_info *xdp_rxq,
			       enum xdp_mem_type type, void *allocator);
void xdp_rxq_info_unreg_mem_model(struct xdp_rxq_info *xdp_rxq);

/* Drivers not supporting XDP metadata can use this helper, which
 * rejects any room expansion for metadata as a result.
 */
static __always_inline void
xdp_set_data_meta_invalid(struct xdp_buff *xdp)
{
	return;
}

static __always_inline bool
xdp_data_meta_unsupported(const struct xdp_buff *xdp)
{
	return false;
}

#endif /* __LINUX_NET_XDP_H__ */
