Summary:	Grep for mailboxes
Name:		mboxgrep
Version:	0.5.3
Release:	1
Epoch:		1
License:	GPL
Group:		Applications/Internet
Source:	    http://public.srce.hr/~dspiljar/mboxgrep-%{version}.tar.gz
URL:        http://public.srce.hr/~dspiljar/mboxgrep.html
BuildRoot:	%{_tmppath}/%{name}-%{version}-root

%description
mboxgrep is a small utility that scans a mailbox for messages 
matching a basic, extended, or Perl-compatible regular 
expression.   Found messages can be either displayed on 
standard output, counted or written to another mailbox.
It supports mbox, MH, nnmh, nnml and maildir folders.

%prep
%setup -q

%build
%configure
%make

%install
rm -rf $RPM_BUILD_ROOT
%makeinstall

%clean
rm -rf $RPM_BUILD_ROOT

%post
/sbin/install-info %{_infodir}/mboxgrep.info* %{_infodir}/dir

%preun
if [ $1 = 0 ]; then
/sbin/install-info --delete %{_infodir}/mboxgrep.info* %{_infodir}/dir
fi

%files
%defattr(-,root,root)
%doc COPYING INSTALL NEWS README
%{_bindir}/*
%{_infodir}/*
%{_mandir}/*/*
