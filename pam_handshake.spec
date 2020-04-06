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
install -m 755 pam_handshake_server %{buildroot}/usr/bin
install -m 755 pam_handshake_auth_check %{buildroot}/usr/bin
install -m 755 pam_handshake_client %{buildroot}/usr/bin


%files
/usr/bin/pam_handshake_server
/usr/bin/pam_handshake_auth_check
/usr/bin/pam_handshake_client

%post
# if [ -e /etc/irods/service_account.config ]
# then
#     source /etc/irods/service_account.config
#     chown $IRODS_SERVICE_ACCOUNT_NAME:$IRODS_SERVICE_GROUP_NAME %{irods_msi_path}/libmsiPidCreate.so
#     chmod 755 %{irods_msi_path}/libmsiPidCreate.so

#     chown $IRODS_SERVICE_ACCOUNT_NAME:$IRODS_SERVICE_GROUP_NAME %{irods_msi_path}/libmsiPidDelete.so
#     chmod 755 %{irods_msi_path}/libmsiPidDelete.so

#     chown $IRODS_SERVICE_ACCOUNT_NAME:$IRODS_SERVICE_GROUP_NAME %{irods_msi_path}/libmsiPidDeleteHandle.so
#     chmod 755 %{irods_msi_path}/libmsiPidDeleteHandle.so

#     chown $IRODS_SERVICE_ACCOUNT_NAME:$IRODS_SERVICE_GROUP_NAME %{irods_msi_path}/libmsiPidLookup.so
#     chmod 755 %{irods_msi_path}/libmsiPidLookup.so

#     chown $IRODS_SERVICE_ACCOUNT_NAME:$IRODS_SERVICE_GROUP_NAME %{irods_msi_path}/libmsiPidLookupOne.so
#     chmod 755 %{irods_msi_path}/libmsiPidLookupOne.so

#     chown $IRODS_SERVICE_ACCOUNT_NAME:$IRODS_SERVICE_GROUP_NAME %{irods_msi_path}/libmsiPidLookupKey.so
#     chmod 755 %{irods_msi_path}/libmsiPidLookupKey.so

#     chown $IRODS_SERVICE_ACCOUNT_NAME:$IRODS_SERVICE_GROUP_NAME %{irods_msi_path}/libmsiPidMove.so
#     chmod 755 %{irods_msi_path}/libmsiPidMove.so

#     chown $IRODS_SERVICE_ACCOUNT_NAME:$IRODS_SERVICE_GROUP_NAME %{irods_msi_path}/libmsiPidMoveHandle.so
#     chmod 755 %{irods_msi_path}/libmsiPidMoveHandle.so

#     chown $IRODS_SERVICE_ACCOUNT_NAME:$IRODS_SERVICE_GROUP_NAME %{irods_msi_path}/libmsiPidGet.so
#     chmod 755 %{irods_msi_path}/libmsiPidGet.so

#     chown $IRODS_SERVICE_ACCOUNT_NAME:$IRODS_SERVICE_GROUP_NAME %{irods_msi_path}/libmsiPidGetHandle.so
#     chmod 755 %{irods_msi_path}/libmsiPidGetHandle.so

#     chown $IRODS_SERVICE_ACCOUNT_NAME:$IRODS_SERVICE_GROUP_NAME %{irods_msi_path}/libmsiPidSet.so
#     chmod 755 %{irods_msi_path}/libmsiPidSet.so

#     chown $IRODS_SERVICE_ACCOUNT_NAME:$IRODS_SERVICE_GROUP_NAME %{irods_msi_path}/libmsiPidSetHandle.so
#     chmod 755 %{irods_msi_path}/libmsiPidSetHandle.so

#     chown $IRODS_SERVICE_ACCOUNT_NAME:$IRODS_SERVICE_GROUP_NAME %{irods_msi_path}/libmsiPidUnset.so
#     chmod 755 %{irods_msi_path}/libmsiPidUnset.so

#     chown $IRODS_SERVICE_ACCOUNT_NAME:$IRODS_SERVICE_GROUP_NAME %{irods_msi_path}/libmsiPidUnsetHandle.so
#     chmod 755 %{irods_msi_path}/libmsiPidUnsetHandle.so

#     chown $IRODS_SERVICE_ACCOUNT_NAME:$IRODS_SERVICE_GROUP_NAME /etc/irods/irods_pid.json.01_default_profile
#     chown $IRODS_SERVICE_ACCOUNT_NAME:$IRODS_SERVICE_GROUP_NAME /etc/irods/irods_pid.json.02_custom_profile

#     chmod 644 /etc/irods/irods_pid.json.01_default_profile
#     chmod 644 /etc/irods/irods_pid.json.02_custom_profile
# fi

%changelog
* Wed Apr 06 2020 Stefan Wolfsheimer <stefan.wolfsheimer@surfsara.nl> - develop
- initial development branch
