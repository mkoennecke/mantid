Name:           mantid-developer
Version:        1.1
Release:        4%{?dist}
Summary:        Meta Package to install dependencies for Mantid Development

Group:          Development/Tools
License:        GPL

BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)


Requires: cmake-gui >= 2.8.5
Requires: boost-devel
Requires: epel-release 
Requires: gperftools-devel
Requires: gperftools-libs
Requires: gcc-c++
Requires: gsl-devel
Requires: hdf-devel
Requires: hdf5-devel
Requires: python-ipython >= 1.1
Requires: muParser-devel
Requires: mxml-devel
Requires: nexus >= 4.2
Requires: nexus-devel >= 4.2
Requires: numpy
Requires: OpenCASCADE-devel
Requires: poco-devel
Requires: PyQt4-devel
Requires: python-devel
Requires: qscintilla-devel
Requires: qt-devel >= 4.6
Requires: qwt-devel
Requires: qwtplot3d-qt4-devel
Requires: redhat-lsb
Requires: rpmdevtools
Requires: scipy
Requires: sip-devel
Requires: git
Requires: openssl-devel
Requires: texlive-latex
Requires: dvipng
Requires: mantidlibs-qt-devel
Requires: mantidlibs-qt-assistant
Requires: scl-utils

BuildArch: noarch

%description
A virtual package which requires all the dependencies and tools that are 
required for Mantid development.

%prep

%build

%install

%clean

%post
# Remove myself once I have installed all the required packages.
#rpm -e %{name}

%files

%changelog
* Tue Feb 04 2014 Stuart Campbell <campbellsi@ornl.gov>
- Added scipy and ipython >= 1.1 dependency

* Tue May 07 2013 Stuart Campbell <campbellsi@ornl.gov>
- Added dvipng and latex for qt-assistant stuff
- Added software collection dependencies

* Thu Jun  7 2012 Russell Taylor <taylorrj@ornl.gov>
- Remove gmock & gtest now that we include them in our repo
- Remove subversion dependency now that we use git

* Mon Mar 19 2012 Stuart Campbell <campbellsi@ornl.gov>
- Updated for google-perftools -> gperftools package rename.

* Wed Feb 22 2012 Stuart Campbell <campbellsi@ornl.gov>
- Added nexus as it is not required by it's devel package.

* Wed Feb 22 2012 Stuart Campbell <campbellsi@ornl.gov>
- Added git as a dependency
- Added openssl-devel dependency

* Mon Feb 20 2012 Stuart Campbell <campbellsi@ornl.gov>
- Added dependency on NeXus development after nexus rpm split.
- Updated CMake dependency to 2.8.5 following 'the virus'!
- Added Google Mock and GTest.
 
* Fri Jun  3 2011 Stuart Campbell <campbellsi@ornl.gov>
- Added rpmdevtools and lsb dependencies

* Fri Jun  3 2011 Stuart Campbell <campbellsi@ornl.gov>
- Added versions for some packages

* Fri Jun  3 2011 Stuart Campbell <campbellsi@ornl.gov>
- Initial release
