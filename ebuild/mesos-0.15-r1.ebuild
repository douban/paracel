# Copyright 1999-2009 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2

EAPI=3

CODE_PROJECT=mesos
CODE_BRANCH="${PVR}"
inherit autotools eutils douban-code

DESCRIPTION="Fine-grained resource sharing and isolation platform for clusters."
HOMEPAGE="http://www.mesosproject.com/"

LICENSE="GPL"
SLOT="0"
KEYWORDS="amd64 x86"
IUSE=""
RESTRICT="mirror"

DEPEND="sys-libs/libunwind >=dev-libs/protobuf-2.4.1[python] app-arch/snappy"
RDEPEND="${DEPEND}"

S="${WORKDIR}/${MY_P}"
LDFLAGS="-lunwind -lsnappy"
CFLAGS="-ggdb"
CXXFLAGS="-ggdb"

src_prepare() {
	cd $S
	./bootstrap
}

src_install() {
	insinto /usr/lib/python2.6/site-packages/
	PP="src/python/build/lib.linux-x86_64-2.6"
	doins ${PP}/mesos_pb2.py ${PP}/mesos.py ${PP}/_mesos.so
	insinto /usr/lib/python2.7/site-packages/
	PP="src/python/build/lib.linux-x86_64-2.7"
	doins ${PP}/mesos_pb2.py ${PP}/mesos.py ${PP}/_mesos.so
	emake DESTDIR="${D}" install || die "emake install failed."
}
