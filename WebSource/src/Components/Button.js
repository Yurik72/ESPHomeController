import React from "react";

class Button extends React.Component {
    render() {
        const dfltclassNames = "btn waves-effect waves-light btn_mode_static";
        const { label,onClick } = this.props;
        const { classNames } = this.props;

        
        return (
        
                <a className={dfltclassNames +" "+ classNames}
                onClick={(ev) => { ev.preventDefault(); onClick(ev) }}
                    href="#"
                    name="action" data-mode="off">{label}
                </a>
           
            );
    }
}
export default Button;