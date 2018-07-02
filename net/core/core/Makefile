
SUBDIR = sox corelib
# common phttp popo tools asyncd smsd sdb session
# dtunnel htunnel 

all-recursive clean-recursive depend-recursive install-recursive release-recursive:
	@target=`echo $@ | sed s/-recursive//`; \
	list='$(SUBDIR)'; \
	for subdir in $$list; do \
		echo "===> Making $$target in $$subdir..."; \
		(cd $$subdir && $(MAKE) $$target) \
	done;
 
all:	all-recursive

clean:	clean-recursive

depend: depend-recursive

release: release-recursive

install: install-recursive copy-file

	
.PHONY : sox
sox:
	make "SUBDIR = sox"

.PHONY : lib
lib:
	make "SUBDIR = corelib"
	
#@(cd common && $(MAKE) common.ra)

