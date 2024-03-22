#[cfg(feature = "qemu")]
pub mod pl011;
#[cfg(feature = "qemu")]
pub use pl011 as uart;
#[cfg(feature = "nxp")]
pub mod imx_uart;
#[cfg(feature = "nxp")]
pub use imx_uart as uart;
pub mod common;
pub mod gicv3;
pub mod pci;
pub mod virtio_trampoline;
