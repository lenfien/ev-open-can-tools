#!/bin/bash

#cat dnsmasq.log | awk '{
#
#    for (i=1; i<=NF; i++) {
#
#        if ($i ~ /tesla/) {
#
#            print $i
#
#        }
#
#    }
#}' | sort -u | awk '{print "server=/" $0 "/223.5.5.5"}' > tesla_domain/tesla_domains_$(date +"%Y%m%d%H%M%S").txt

cat dnsmasq.log | grep query | grep 172.16.0.229 | awk '{print $6}' | sort -u | grep 'tesla'| awk '{print "server=/" $0 "/223.5.5.5"}' > tesla_domain/tesla_domains_$(date +"%Y%m%d%H%M%S").txt

scp -rp dnsmasq.log tesla_domain/* hongbozhang@172.16.0.153:~/tesla_domain/

