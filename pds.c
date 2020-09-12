#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<stdlib.h>
#include<assert.h>
#include "bst.h"
// #include "bst.c"
#include "pds.h"

struct PDS_RepoInfo repo_handle;

int deleted_record_offset[105];

int findfileSize(char f_n[]) {
   FILE* fp = (FILE *)fopen(f_n, "r"); // opening a file in read mode
   fseek(fp, 0L, SEEK_END);
   int res = ftell(fp); //counting the size of the file
   fclose(fp); //closing the file
   return res;
}

int isEmpty(FILE *file){
    long savedOffset = ftell(file);
    fseek(file, 0, SEEK_END);

    if (ftell(file) == 0){
        return 1;
    }

    fseek(file, savedOffset, SEEK_SET);
    return 0;
}

int pds_open(char *repo_name,int rec_size)
{
    // assert(rec_size==sizeof(struct PDS_RepoInfo));
    char repo_file[30];
    char ndx_file[30];

    if(repo_handle.repo_status==PDS_REPO_OPEN)
        return PDS_REPO_ALREADY_OPEN;
    

    strcpy(repo_handle.pds_name,repo_name);

    strcpy(repo_file,repo_name);
    strcat(repo_file,".dat");
    strcpy(ndx_file,repo_name);
    strcat(ndx_file,".ndx");

    repo_handle.pds_data_fp=(FILE *)fopen(repo_file,"rb+");
    if(repo_handle.pds_data_fp==NULL)
        perror(repo_file);

    repo_handle.pds_ndx_fp=(FILE *)fopen(ndx_file,"rb");
    if(repo_handle.pds_ndx_fp==NULL)
        perror(ndx_file);
    
    repo_handle.repo_status=PDS_REPO_OPEN;
    repo_handle.rec_size=rec_size;
    repo_handle.pds_bst=NULL;
    
    /*added from here*/
    
    struct PDS_NdxInfo *read_data;
    
    if(isEmpty(repo_handle.pds_ndx_fp))
        memset(deleted_record_offset,-1,sizeof(deleted_record_offset));
    else
        fread(deleted_record_offset,sizeof(int),100,repo_handle.pds_ndx_fp);
    
    while(feof(repo_handle.pds_ndx_fp)==0)
    {
        read_data=(struct PDS_NdxInfo *)malloc(sizeof(struct PDS_NdxInfo));
        fread(read_data,sizeof(struct PDS_NdxInfo),1,repo_handle.pds_ndx_fp);
        bst_add_node(&repo_handle.pds_bst,read_data->key,read_data);
    }
    /*till here*/
    return PDS_SUCCESS;
}

int put_rec_by_key(int key,void *rec)
{    
    int offset,status,writesize;
    struct PDS_NdxInfo *ndx_entry;
    fseek(repo_handle.pds_data_fp,0,SEEK_END);
    offset=ftell(repo_handle.pds_data_fp);
    int ind=-1;
    for(int i=0;i<100;i++)
    {
        if(deleted_record_offset[i]>=0)
        {
            ind=i;
            offset=deleted_record_offset[i];
            break;
        }
    }

    ndx_entry=(struct PDS_NdxInfo *)malloc(sizeof(struct PDS_NdxInfo));
    ndx_entry->key=key;
    ndx_entry->offset=offset;
    int bst_status=bst_add_node(&repo_handle.pds_bst,key,ndx_entry);
    if(bst_status!=BST_SUCCESS){
        free(ndx_entry);
        return PDS_ADD_FAILED;
    }
    
    if(ind>=0)
        deleted_record_offset[ind]=-1;
    
    fseek(repo_handle.pds_data_fp,offset,SEEK_SET);
    fwrite(&key,sizeof(int),1,repo_handle.pds_data_fp);
    fwrite(rec, repo_handle.rec_size, 1, repo_handle.pds_data_fp);
    return PDS_SUCCESS;    
}

int get_rec_by_ndx_key(int key, void *rec)
{
    int offset;
    struct BST_Node *temp_node = bst_search(repo_handle.pds_bst, key);
    if (temp_node == NULL)
        return PDS_REC_NOT_FOUND;
    
    else{
        struct PDS_NdxInfo *ndx_entry = (struct PDS_NdxInfo *)(temp_node->data);
        offset = ndx_entry->offset;
        fseek(repo_handle.pds_data_fp, offset, SEEK_SET);
        int tmpkey;
        fread(&tmpkey,sizeof(int),1,repo_handle.pds_data_fp);
        fread(rec, repo_handle.rec_size, 1, repo_handle.pds_data_fp);
        return PDS_SUCCESS;
    }
}

void bst_preorder(struct BST_Node *roott)
{
    if(roott==NULL)
    {
        return;
    }
    fwrite((struct PDS_NdxInfo*)(roott->data),sizeof(struct PDS_NdxInfo),1,repo_handle.pds_ndx_fp);
    bst_preorder(roott->left_child);
    bst_preorder(roott->right_child);
}

int pds_close()
{
    /*added from here*/
    char ndx_file[30];
    strcpy(ndx_file,repo_handle.pds_name);
    strcat(ndx_file,".ndx");
    fclose(repo_handle.pds_ndx_fp);
    repo_handle.pds_ndx_fp=(FILE*)fopen(ndx_file,"wb");
    fwrite(deleted_record_offset,sizeof(int),100,repo_handle.pds_ndx_fp);
    bst_preorder(repo_handle.pds_bst);
    fclose(repo_handle.pds_ndx_fp);

    strcpy(repo_handle.pds_name, "");
    fclose(repo_handle.pds_data_fp);
    /*till here*/

    bst_destroy(repo_handle.pds_bst);
    repo_handle.repo_status = PDS_REPO_CLOSED;
    return PDS_SUCCESS;
}

int get_rec_by_non_ndx_key( 
void *key,  			/* The search key */
void *rec,  			/* The output record */
int (*matcher)(void *rec, void *key), /*Function pointer for matching*/
int *io_count  		/* Count of the number of records read */
)
{
    fseek(repo_handle.pds_data_fp,0L,SEEK_SET);

    *io_count=0;
    while(feof(repo_handle.pds_data_fp)==0)
    {
        // struct PDS_RepoInfo *cur;
        int curoffset=ftell(repo_handle.pds_data_fp);
        int flag=1;
        for(int j=0;j<100;j++)
        {
            if(deleted_record_offset[j]==curoffset)
            {
                flag=0;break;
            }
        }
        
        int tmpkey;
        fread(&tmpkey,sizeof(int),1,repo_handle.pds_data_fp);
        fread(rec,repo_handle.rec_size,1,repo_handle.pds_data_fp);
        

        if(!flag)continue;
        (*io_count)++;
        int er=matcher(rec,key);
        
        if(er>1)return er;
        else if(er==0)return 0;
    }
    rec=NULL;
    return 0;   
}

int update_by_key( int key, void *newrec )
{
    int offset;
    struct BST_Node *temp_node = bst_search(repo_handle.pds_bst, key);
    if (temp_node == NULL)
        return PDS_REC_NOT_FOUND;
    
    else{
        struct PDS_NdxInfo *ndx_entry = (struct PDS_NdxInfo *)(temp_node->data);
        offset = ndx_entry->offset;
        fseek(repo_handle.pds_data_fp, offset, SEEK_SET);
        fwrite(&key,sizeof(int),1,repo_handle.pds_data_fp);
        fwrite(newrec, repo_handle.rec_size, 1, repo_handle.pds_data_fp);
        return PDS_SUCCESS;
    }
}   

int delete_by_key( int key )
{
    int offset;
    struct BST_Node *temp_node = bst_search(repo_handle.pds_bst, key);
    if (temp_node == NULL)
        return PDS_REC_NOT_FOUND;
    else
    {
        struct PDS_NdxInfo *ndx_entry = (struct PDS_NdxInfo *)(temp_node->data);
        offset = ndx_entry->offset;
        int flag=0;
        for(int i=0;i<100;i++)
        {
            if(deleted_record_offset[i]==-1)
            {
                deleted_record_offset[i]=offset;flag=1;
                break;
            }
        }
        if(!flag)return PDS_FILE_ERROR;
        bst_del_node(&repo_handle.pds_bst,key);
        return PDS_SUCCESS;
    }    
}