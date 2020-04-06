Name:           %{packagename}
Version:        %{version}
Release:        1%{?dist}
Summary:        Integration of iRODS and handle system via microservices.

License:        GPLv3+
Source0:        %{packagename}-%{version}.tar.gz

# %if ( "%{irodsversion}" == "4.1.11" || "%{irodsversion}" == "4.1.12" )
# %define irods_msi_path /var/lib/irods/plugins/microservices
# %else
# %define irods_msi_path /usr/lib/irods/plugins/microservices
# %endif


# %define libs_dir lib/_%{irodsversion}
# %define irods_config_path /etc/irods
# %define debug_package %{nil}

%description
Creating and managing persistent identifiers (handle) with iRods microservices.

%prep
%setup -q


%build
make

%install

cp server pam_handshake_server
cp auth_check pam_handshake_auth_check

mkdir -p %{buildroot}/usr/bin
mkdir -p %{buildroot}/etc/systemd/system/
install -m 755 pam_handshake_server %{buildroot}/usr/bin
install -m 755 pam_handshake_auth_check %{buildroot}/usr/bin
install -m 755 pam-handshake.service %{buildroot}/etc/systemd/system/
install -m 755 pam_handshake_start.sh %{buildroot}/usr/bin
install -m 755 pam_handshake_status.sh %{buildroot}/usr/bin
install -m 755 pam_handshake_stop.sh %{buildroot}/usr/bin

%files
/usr/bin/pam_handshake_server
/usr/bin/pam_handshake_auth_check
/etc/systemd/system/pam-handshake.service
/usr/bin/pam_handshake_start.sh
/usr/bin/pam_handshake_status.sh
/usr/bin/pam_handshake_stop.sh

%post

%changelog
* Wed Apr 06 2020 Stefan Wolfsheimer <stefan.wolfsheimer@surfsara.nl> - develop
- initial development branch
