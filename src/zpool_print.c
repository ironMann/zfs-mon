
#include "zpool.h"
#include "zpool_prop.h"


// void zfsmon_print_prop(const zfsmon_prop_t *prop)
// {
//     printf("%-40s", prop->name);
//     switch (prop->type)
//     {
//     case string_prop:
//         if (NULL != prop->v.s )
//             printf("%s\n", prop->v.s);
//         else
//             printf("NaN\n");
//         break;
//     case int_prop:
//         printf("%" PRIi64 "\n", prop->v.i);
//         break;
//     case uint_prop:
//         printf("%" PRIu64 "\n", prop->v.u);
//         break;
//     case double_prop:
//         printf("%lf\n", prop->v.d);
//         break;
//     default:
//         printf("XXXX\n");
//         break;
//     }
// }


// void zfsmon_print_zpool_props(const zfsmon_zpool_t *p)
// {
//     size_t i;

//     printf("%-40s\n", p->name);
//     printf("------------------------------------\n");

//     /* print known properties */
//     for (i = 0; i < zfsmon_zpool_pops_count(); i++)
//     {
//         zfsmon_print_prop(&p->properties.props[i]);
//     }

//     /* print additional properties */
//     for (i = 0; i < p->properties.u_count; i++)
//     {
//         printf("%-40s%s\n", p->properties.u_prop[i].prop_name, p->properties.u_prop[i].prop_val);
//     }
// }
