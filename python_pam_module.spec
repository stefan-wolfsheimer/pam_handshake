Name:           %{packagename}
Version:        %{version}
Release:        %{release}
Summary:        Integration of iRODS and handle system via microservices.

License:        GPLv3+
Source0:        %{packagename}-%{version}.tar.gz

Requires: python-libs, python-devel, python3-devel, python3-libs

%description
Python PAM module

%prep
%setup -q

%build
make -C src_module

%install
mkdir -p %{buildroot}/lib64/security
cp src_module/pam_python3.so %{buildroot}/lib64/security/pam_python3.so
cp src_module/pam_python.so %{buildroot}/lib64/security/pam_python.so

%files
/lib64/security/pam_python3.so
/lib64/security/pam_python.so

%post

%changelog
* Mon May 15 2020 Stefan Wolfsheimer <stefan.wolfsheimer@surfsara.nl>
- initial repackaged version
