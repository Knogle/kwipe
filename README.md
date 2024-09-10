# kwipe
[![ci_formatting](https://github.com/Knogle/nwipe/actions/workflows/ci_formatting.yml/badge.svg)](https://github.com/Knogle/nwipe/actions/workflows/ci_formatting.yml)

## Reasons for forking nwipe

As there are requirements in Germany for BSI TR-03109 compliant erasure, this is a fork from https://github.com/martijnvanbrummelen/nwipe called kwipe (**k**ryptowipe).

BSI TR-02102, titled **"Cryptographic Mechanisms: Recommendations and Key Lengths"**, provides guidelines and recommendations for the use of cryptographic mechanisms in various security applications in Germany. It covers aspects such as algorithms, key lengths, and specific requirements for cryptographic primitives.

### Key points related to **CSPRNG (Cryptographically Secure Pseudo-Random Number Generators)**:

1. **Security Requirements**: The TR-02102 emphasizes that any PRNG used in cryptographic applications must be cryptographically secure (CSPRNG), meaning it should be unpredictable and resistant to attacks, even if parts of its internal state are known. This ensures that the output cannot be predicted by attackers.

2. **Entropy**: A CSPRNG must have a strong source of entropy. The higher the entropy, the more difficult it is for an attacker to predict future outputs of the generator.

3. **Algorithm Recommendations**: The BSI recommends specific algorithms that are considered cryptographically secure, such as:
   - **AES-based DRBG** (Deterministic Random Bit Generator)
   - **Hash-based DRBGs** like those defined by NIST (e.g., HMAC_DRBG)
   - **Elliptic Curve-based DRBGs**

4. **Compliance with Standards**: CSPRNGs must comply with international standards such as NIST SP 800-90A or other equivalent guidelines that ensure their robustness against known cryptographic attacks.

In short, BSI TR-02102 mandates that CSPRNGs be highly unpredictable, have a strong source of entropy, and comply with well-established cryptographic standards to ensure secure generation of random numbers, crucial for secure data erasure, encryption, and other security-sensitive operations.
BSI TR-02102 emphasizes that CSPRNGs must withstand rigorous audits and meet high security standards. Therefore, using well-established libraries like OpenSSL is preferable for ensuring compliance and reliability.

## Description

kwipe is a fork of the dwipe command originally used by Darik's Boot and Nuke (DBAN). kwipe was created out of a need to run the DBAN dwipe command outside of DBAN, in order to allow its use with any host distribution, thus giving better hardware support.

kwipe is a program that will securely erase the entire contents of disks. It can wipe a single drive or multiple disks simultaneously. It can operate as both a command line tool without a GUI or with a ncurses GUI as shown in the example below:

> **Warning**
> For some of kwipes features such as smart data in the PDF certificate, HPA/DCO detection and other uses, kwipe utilises smartmontools and hdparm. Therefore both hdparm & smartmontools are a mandatory requirement if you want all of kwipes features to be fully available. If you do not install smartmontools and hdparm, kwipe will provide a warning in the log that these programs cannot be found but will still run but many important features may not work as they should do.

![Example wipe](/images/output.gif)

<i>The video above shows six drives being simultaneously erased. It skips to the completion of all six wipes and shows five drives that were successfully erased and one drive that failed due to an I/O error. The drive that failed would then normally be physically destroyed. The five drives that were successfully wiped with zero errors or failures can then be redeployed.</i>

![kwipe_certificate_0 35_5s](https://github.com/martijnvanbrummelen/nwipe/assets/22084881/cf181a9c-af2d-4bca-a6ed-15a4726cb12b)
<i>The snapshot above shows nwipe's three page PDF certificate, drive identifiable information such as serial numbers have been anonymised using the kwipe command line option -q</i>

## Erasure methods
The user can select from a variety of recognised secure erase methods which include:

* Fill With Zeros    - Fills the device with zeros (0x00), one round only.
* Fill With Ones     - Fills the device with ones  (0xFF), one round only.
* RCMP TSSIT OPS-II  - Royal Candian Mounted Police Technical Security Standard, OPS-II
* DoD Short          - The American Department of Defense 5220.22-M short 3 pass wipe (passes 1, 2 & 7).
* DoD 5220.22M       - The American Department of Defense 5220.22-M full 7 pass wipe.
* Gutmann Wipe       - Peter Gutmann's method (Secure Deletion of Data from Magnetic and Solid-State Memory).
* PRNG Stream        - Fills the device with a stream from the PRNG.
* Verify Zeros       - This method only reads the device and checks that it is filled with zeros (0x00).
* Verify Ones        - This method only reads the device and checks that it is filled with ones (0xFF).
* HMG IS5 enhanced   - Secure Sanitisation of Protectively Marked Information or Sensitive Information

kwipe also includes the following pseudo random number generators (prng):
* Mersenne Twister
* ISAAC
* XORoshiro-256
* Lagged Fibonacci
* AES-CTR (openssl)

These can be used to overwrite a drive with a stream of randomly generated characters.

kwipe is also included in [ShredOS](https://github.com/Knogle/shredos.x86_64) which was developed in particular to showcase kwipe as a fast-to-boot standalone method similar to DBAN. ShredOS always contains the latest kwipe version.

## Limitations regarding solid state drives
In the current form kwipe does not sanitize solid state drives (hereinafter referred to as SSDs)
of any form (SAS / Sata / NVME) and / or form factor (2.5" / 3.5" / PCI) fully due to their nature:  
SSDs, as the transistors contained in the memory modules are subject to wear, contain in most cases
additional memory modules installed as failover for broken sectors outside
of the host accessible space (frequently referred to as "overprovisioning") and for garbage collection.
Some manufacturers reserve access to these areas only to disk's own controller and firmware.
It is therefor always advised to use kwipe / shredOS in conjunction with the manufacturer's or hardware vendor's own tools for sanitization to assure
full destruction of the information contained on the disk.
Given that most vendors and manufacturers do not provide open source tools, it is advised to validate the outcome by comparing the data on the disk before and after sanitization.
A list of the most common tools and instructions for SSD wipes can be found in the [SSD Guide](ssd-guide.md).

## Compiling & Installing

For a development setup, see the [Hacking section](#hacking) below. For a bootable version of the very latest kwipe master that you can write to an USB flash drive or CD/DVD, see the [Quick and easy bootable version of kwipe master section](#quick--easy-usb-bootable-version-of-kwipe-master-for-x86_64-systems) below.

`kwipe` requires the following libraries to be installed:

* ncurses
* pthreads
* parted
* libconfig
* openssl

`kwipe` also requires the following program to be installed, it will abort with a warning if not found:

* hdparm (as of current master and v0.35+)

and optionally, but recommended, the following programs:

* dmidecode
* readlink
* smartmontools

### Debian & Ubuntu prerequisites

If you are compiling `kwipe` from source, the following libraries will need to be installed first:

```bash
sudo apt install \
  build-essential \
  pkg-config \
  automake \
  libncurses5-dev \
  autotools-dev \
  libparted-dev \
  libconfig-dev \
  libconfig++-dev \
  dmidecode \
  coreutils \
  smartmontools \
  hdparm \
  libssl-dev
```

### Fedora prerequisites

```bash
sudo bash
dnf update
dnf groupinstall "Development Tools"
dnf groupinstall "C Development Tools and Libraries"
yum install ncurses-devel
yum install parted-devel
yum install libconfig-devel
yum install libconfig++-devel
yum install dmidecode
yum install coreutils
yum install smartmontools
yum install hdparm
yum install libssl-devel
```
Note. The following programs are optionally installed although recommended. 1. dmidecode 2. readlink 3. smartmontools.

#### hdparm [REQUIRED]
hdparm provides some of the information regarding disk size in sectors as related to the host protected area (HPA) and device configuration overlay (DCO). We do however have our own function that directly access the DCO to obtain the 'real max sectors' so reliance on hdparm may be removed at a future date.

#### dmidecode [RECOMMENDED]
dmidecode provides SMBIOS/DMI host data to stdout or the log file. If you don't install it you won't see the SMBIOS/DMI host data at the beginning of kwipes log.

#### coreutils (provides readlink) [RECOMMENDED]
readlink determines the bus type, i.e. ATA, USB etc. Without it the --nousb option won't work and bus type information will be missing from kwipes selection and wipe windows. The coreutils package is often automatically installed as default in most if not all distros.

#### smartmontools [REQUIRED]
smartmontools obtains serial number information for supported USB to IDE/SATA adapters. Without it, drives plugged into USB ports will not show serial number information.

If you want a quick and easy way to keep your copy of kwipe running the latest master release of kwipe see the [automating the download and compilation](#automating-the-download-and-compilation-process-for-debian-based-distros) section.

### Compilation

First create all the autoconf files:
```
./autogen.sh
```

Then compile & install using the following standard commands:
```
./configure
make format (only required if submitting pull requests)
make
make install
```

Then run kwipe !
```
cd src
sudo ./kwipe
```
or
```
sudo kwipe
```

### Hacking

If you wish to submit pull requests to this code we would prefer you enable all warnings when compiling.
This can be done using the following compile commands:

```
./configure --prefix=/usr CFLAGS='-O0 -g -Wall -Wextra'
make format (necessary if submitting pull requests)
make
make install
```

The `-O0 -g` flags disable optimisations. This is required if you're debugging with `gdb` in an IDE such as Kdevelop. With these optimisations enabled you won't be able to see the values of many variables in kwipe, not to mention the IDE won't step through the code properly.

The `-Wall` and `-Wextra` flags enable all compiler warnings. Please submit code with zero warnings.

Also make sure that your changes are consistent with the coding style defined in the `.clang-format` file, using:
```
make format
```
You will need `clang-format` installed to use the `format` command.

Once done with your coding then the released/patch/fixed code can be compiled, with all the normal optimisations, using:
```
./configure --prefix=/usr && make && make install
```

## Automating the download and compilation process for Debian based distros.

Here's a script that will do just that! It will create a directory in your home folder called 'kwipe_master'. It installs all the libraries required to compile the software (build-essential) and all the libraries that kwipe requires (libparted etc). It downloads the latest master copy of kwipe from github. It then compiles the software and then runs the latest kwipe. It doesn't write over the version of kwipe that's installed in the repository (If you had kwipe already installed). To run the latest master version of kwipe manually you would run it like this `sudo ~/kwipe_master/kwipe/src/kwipe`

You can run the script multiple times; the first time it's run it will install all the libraries; subsequent times it will just say the libraries are up to date. As it always downloads a fresh copy of the kwipe master from Github, you can always stay up to date. Just run it to get the latest version of kwipe. It only takes 11 seconds to compile on my i7.

If you already have kwipe installed from the repository, you need to take care which version you are running. If you typed `kwipe` from any directory it will always run the original repository copy of kwipe. To run the latest kwipe you have to explicitly tell it where the new copy is, e.g in the directory `~/kwipe_master/kwipe/src`. That's why you would run it by typing `sudo ~/kwipe_master/kwipe/src/kwipe` alternatively you could cd to the directory and run it like this:

```
cd ~/kwipe_master/kwipe/src
./kwipe
```

Note the ./, that means only look in the current directory for kwipe. if you forgot to type ./ the computer would run the older repository version of kwipe.

Once you have copied the script below into a file called buildkwipe, you need to give the file execute permissions `chmod +x buildkwipe` before you can run it.
```
#!/bin/bash
cd "$HOME"
kwipe_directory="kwipe_master"
mkdir $kwipe_directory
cd $kwipe_directory
sudo apt install build-essential pkg-config automake libncurses5-dev autotools-dev libparted-dev libconfig-dev libconfig++-dev dmidecode readlink smartmontools git
rm -rf kwipe
git clone https://github.com/martijnvanbrummelen/kwipe.git
cd "kwipe"
./autogen.sh
./configure
make
cd "src"
sudo ./kwipe
```

## Quick & Easy, USB bootable version of kwipe master for x86_64 systems.
If you want to just try out a bootable version of kwipe you can download [ShredOS](https://github.com/Knogle/shredos.x86_64). The ShredOS image is around 60MB and can be written to an USB flash drive in seconds, allowing you to boot straight into the latest version of kwipe. ShredOS is available for x86_64 (64-bit) and i686 (32-bit) CPU architectures and will boot both legacy BIOS and UEFI devices. It comes as .IMG (bootable USB flash drive image) or .ISO (for CD-R/DVD-R). Instructions and download can be found [here](https://github.com/Knogle/shredos.x86_64#obtaining-and-writing-shredos-to-a-usb-flash-drive-the-easy-way-).

## License

GNU General Public License v2.0
